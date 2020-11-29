#include <stdio.h>
#include "block.h"
#include "wavetable.h"
#include "util.h"

void Wavetable::grab(float *buf) {
    for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
        *buf++ = wt_sample(wt);
        wt_walk(wt);
    }
}

void Gate::grab(float *buf) {
    float A[FRAMES_PER_BUFFER] = {0};
    float B[FRAMES_PER_BUFFER] = {0};

    try_grab_from_parent(0, A);
    try_grab_from_parent(1, B);
    
    for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
        *buf++ = A[i] * B[i];
    }
}

void Trigger::grab(float *buf) {
    for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            *buf++ = this->state;
        }
}

void Mixer::grab(float *buf) {
    float A[FRAMES_PER_BUFFER];
    float B[FRAMES_PER_BUFFER];
    float C[FRAMES_PER_BUFFER];

    try_grab_from_parent(0, A);
    try_grab_from_parent(1, B);
    try_grab_from_parent(2, C);

    for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
        *buf++ = this->gain*(A[i]*B[i]*C[i]);
    }
}

void Envelope::grab(float *buf) {
    float A[FRAMES_PER_BUFFER];
    try_grab_from_parent(0, A);
    
    for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
        if (this->previous == 0.0 && A[i] == 1.0) {
            // rising edge detected
            this->state = ENV_A;
            this->sample_counter = 0;
            this->ref = this->previous_out;
        } else if (this->previous == 1.0 && A[i] == 0.0) {
            // falling edge detected
            this->state = ENV_R;
            this->sample_counter = 0;
            this->ref = this->previous_out;
        }
        if (this->state == ENV_A) {
            float attack_doneness = (float)this->sample_counter / (float)this->attack;
            this->previous_out = lerp(this->ref, 1, attack_doneness);
            *buf++ = this->previous_out;
            this->sample_counter++;
            if (this->sample_counter > this->attack) {
                this->state = ENV_D;
                this->sample_counter = 0;
                this->ref = 1.0;
            }
        } else if (this->state == ENV_D) {
            float decay_doneness = (float)this->sample_counter / (float)this->decay;
            this->previous_out = lerp(this->ref, this->sustain, decay_doneness);
            *buf++ = this->previous_out;
            this->sample_counter++;
            if (this->sample_counter > this->decay) {
                this->state = ENV_S;
                this->sample_counter = 0;
                this->ref = this->sustain;
            }
        } else if (this->state == ENV_S) {
            this->previous_out = this->sustain;
            *buf++ = this->previous_out;
        } else if (this->state == ENV_R) {
            float release_doneness = (float)this->sample_counter/(float)this->release;
            this->previous_out = lerp(this->ref, 0, release_doneness);
            *buf++ = this->previous_out;
            this->sample_counter++;
            if (this->sample_counter > this->release) {
                this->state = ENV_OFF;
                this->sample_counter = 0;
            }
        } else if (this->state == ENV_OFF) {
            *buf++ = 0;
        }
        this->previous = A[i];
    }
}

void Sequencer::grab(float *buf) {
    for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
        //printf("i: %d, sn: %d\n", i, b->data.seq.sample_num);
        if (this->sample_num >= this->samples_per_division * NUM_SEQ_DIVISIONS) {
            //printf("seq reset\n");
            this->sample_num = 0;
        }
        //printf("%d\n",  b->data.seq.samples_per_division);
        int seq_index = this->sample_num / this->samples_per_division; 
        *buf++ = this->value[seq_index];
        this->sample_num++;
    }
}