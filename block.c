#include "block.h"
#include "wavetable.h"
#include "util.h"

void grab(block *b, float *buf) {
    if (b->type == BLOCK_WAVETABLE) {
        wavetable *wt = b->data.wt;
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            *buf++ = wt_sample(wt);
            wt_walk(wt);
        }
    } else if (b->type == BLOCK_GATE) {
        float A[FRAMES_PER_BUFFER];
        float B[FRAMES_PER_BUFFER];
        grab(b->parents[0], A);
        grab(b->parents[1], B);
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            *buf++ = A[i] * B[i];
        }
    } else if (b->type == BLOCK_TRIGGER) {
        // trigger value is 0 or 1 stored in data field
        // probably instead it should be a sdl_keycode and ptr to the map (or map is global)
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            *buf++ = b->data.constant;
        }
    } else if (b->type == BLOCK_CONSTANT) {
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            *buf++ = b->data.constant;
        }
    } else if (b->type == BLOCK_MIXER) {
        float A[FRAMES_PER_BUFFER];
        float B[FRAMES_PER_BUFFER];
        float C[FRAMES_PER_BUFFER];
        grab(b->parents[0], A);
        grab(b->parents[1], B);
        grab(b->parents[2], C);
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            *buf++ = 0.5*A[i] + 0.5*B[i] + 0.5*C[i];
        }
    } else if (b->type == BLOCK_ENVELOPE) {
        float A[FRAMES_PER_BUFFER];
        grab(b->parents[0], A);
        envelope *e = &b->data.env;
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            if (e->previous == 0.0 && A[i] == 1.0) {
                // rising edge detected
                e->state = ENV_A;
                e->sample_counter = 0;
                e->ref = e->previous_out;
            } else if (e->previous == 1.0 && A[i] == 0.0) {
                // falling edge detected
                e->state = ENV_R;
                e->sample_counter = 0;
                e->ref = e->previous_out;
            }
            if (e->state == ENV_A) {
                float attack_doneness = (float)e->sample_counter / (float)e->attack;
                e->previous_out = lerp(e->ref, 1, attack_doneness);
                *buf++ = e->previous_out;
                e->sample_counter++;
                if (e->sample_counter > e->attack) {
                    e->state = ENV_D;
                    e->sample_counter = 0;
                    e->ref = 1.0;
                }
            } else if (e->state == ENV_D) {
                float decay_doneness = (float)e->sample_counter / (float)e->decay;
                e->previous_out = lerp(e->ref, e->sustain, decay_doneness);
                *buf++ = e->previous_out;
                e->sample_counter++;
                if (e->sample_counter > e->decay) {
                    e->state = ENV_S;
                    e->sample_counter = 0;
                    e->ref = e->sustain;
                }
            } else if (e->state == ENV_S) {
                e->previous_out = e->sustain;
                *buf++ = e->previous_out;
            } else if (e->state == ENV_R) {
                float release_doneness = (float)e->sample_counter/(float)e->release;
                e->previous_out = lerp(e->ref, 0, release_doneness);
                *buf++ = e->previous_out;
                e->sample_counter++;
                if (e->sample_counter > e->release) {
                    e->state = ENV_OFF;
                    e->sample_counter = 0;
                }
            } else if (e->state == ENV_OFF) {
                *buf++ = 0;
            }
            e->previous = A[i];
        }
    }
}

