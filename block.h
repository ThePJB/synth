#ifndef BLOCK_H
#define BLOCK_H

#include "wavetable.h"
#include "ringbuf.h"
#include "string.h"
#include "graphics.h"
#include "message_queue.h"
#include <stdint.h>
#include <SDL.h>


#define MAX_PARENTS 4
#define FRAMES_PER_BUFFER 256

struct FailGrabMessage: Message {
    Block *parent;
    Block *child;

    FailGrabMessage(Block *parent, Block *child) {
        this->parent = parent;
        this->child = child;
    }

    virtual void print() {
        printf("Failed to grab from %s to %s\n", parent, child);
    }
}

struct Block {
    int x = 0;
    int y = 0;
    int w = 100;
    int h = 100;
    uint8_t r = 40;
    uint8_t g = 40;
    uint8_t b = 40;
    Block *parents[MAX_PARENTS] = {0};
    Block *child = 0;
    float avg_volume;
    char *name;

    SDL_Rect get_rect() {
        return SDL_Rect{this->x, this->y, this->w, this->h};
    }

    void compute_avg(float *buf) {
        this->avg_volume = 0.5;
        /*
        float avg = 0;
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            avg += (buf[i] * buf[i]);
        }
        this->avg_volume = avg / FRAMES_PER_BUFFER;
        */
    }

    static void vu_meter(SDL_Rect r, float avg) {
        // assume max of 1
        int bar_h = r.h;
        int h = int(r.h * avg);

        auto g = Graphics::get();
        SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(g->renderer, &r);
        SDL_SetRenderDrawColor(g->renderer, 255, 0, 0, 255);
        r.h = h;
        r.y = r.y + (bar_h - h);
        //printf("vu avg %f h: %d y: %d\n", avg, r.h, r.y);
        SDL_RenderFillRect(g->renderer, &r);
    }

    // call this
    void grab_next(float *buf) {
        // specific actions
        this->grab_next_impl(buf);

        // shared actions
        this->compute_avg(buf);
    }

    // implement this
    virtual void grab_next_impl(float *buf) = 0;
    virtual void draw() {
        auto g = Graphics::get();
        SDL_SetRenderDrawColor(g->renderer, this->r, this->g, this->b, 255);
        SDL_RenderFillRect(g->renderer, &this->get_rect());

        // draw child connection
        if (this->child != 0) {
            SDL_SetRenderDrawColor(g->renderer, 255, 0, 0, 255);
            SDL_RenderDrawLine(g->renderer, this->x + this->w, this->y, this->child->x, this->child->y);
        }
    }

    void try_grab_from_parent(int index, float *buf) {
        Block *b;
        if ((b = this->parents[index]) != 0) {
            b->grab_next(buf);
        } else {
            MessageQueue::get()->push(FailGrabMessage(this->parents[index], this));
        }
    }

    void set_pos(int x, int y) {
        this->x = x;
        this->y = y;
    }
};

struct Wavetable: Block {
    wavetable *wt;
    Wavetable(wavetable *wt) { this->wt = wt; }
    void grab_next_impl(float *buf) {
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            *buf++ = wt_sample(wt);
            wt_walk(wt);
        }
    }
};

struct Gate: Block {
    Gate() {};
    void grab_next_impl(float *buf) {
        float A[FRAMES_PER_BUFFER] = {0};
        float B[FRAMES_PER_BUFFER] = {0};

        try_grab_from_parent(0, A);
        try_grab_from_parent(1, B);
        
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            *buf++ = A[i] * B[i];
        }        
    }
};

struct Trigger: Block {
    float state;

    Trigger() {
        state = 0;
    }
    virtual void draw() {
        Block::draw();
        auto g = Graphics::get();
        SDL_Rect indicator = SDL_Rect{this->x + 8, this->y + 8, 8, 8};
        if (state == 1) {
            SDL_SetRenderDrawColor(g->renderer, 255,0,0,255);
        } else {
            SDL_SetRenderDrawColor(g->renderer, 0,0,0,255);
        }
        SDL_RenderFillRect(g->renderer, &indicator);

    }

    void set_trigger() { this->state = 1; }
    void unset_trigger() { this->state = 0; }
    void grab_next_impl(float *buf) {
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
                *buf++ = this->state;
        }
    }
};

struct Mixer: Block {
    float gain;

    Mixer(float gain) {
        this->gain = gain;
    }
    void grab_next_impl(float *buf) {
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

    virtual void draw() {
        auto g = Graphics::get();
        SDL_SetRenderDrawColor(g->renderer, 255,255,255,255);
        SDL_RenderFillRect(g->renderer, &this->get_rect());
    }
};

struct Envelope: Block {
    typedef enum {
        ENV_A,
        ENV_D,
        ENV_S,
        ENV_R,
        ENV_OFF,
    } envelope_state;

    envelope_state state;
    uint32_t sample_counter;
    float previous;

    uint32_t attack; // num samples
    uint32_t decay; // num samples
    float sustain; // sustain volume
    uint32_t release; // num samples

    float ref;  // last point we are coming down from
    float previous_out;

    Envelope(uint32_t attack, uint32_t decay, float sustain, uint32_t release) {
        this->state = ENV_OFF;
        this->sample_counter = 0;
        this->previous = 0;

        this->attack = attack;
        this->decay = decay;
        this->sustain = sustain;
        this->release  = release;

        this->ref = 0; // diff with previous?
        this->previous_out = 0; // used?
    }
    void grab_next_impl(float *buf) {
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
    
    // todo make this draw where the envelopes at thinkin just horizontal line. or even better if you could visualize the shape of the envelope
    // show if its held with colour maybe
    virtual void draw() {
        int bar_height = this->h - (int)(this->h * this->previous_out);
        
        Block::draw();
        auto g = Graphics::get();
        SDL_Rect indicator = SDL_Rect{this->x, this->y-2 + bar_height, this->w, 4};
        if (this->state == ENV_A) {
            SDL_SetRenderDrawColor(g->renderer, 255,0,0,255);
        } else if (this->state == ENV_D) {
            SDL_SetRenderDrawColor(g->renderer, 255,255,0,255);
        } else if (this->state == ENV_S) {
            SDL_SetRenderDrawColor(g->renderer, 0,255,0,255);
        } else if (this->state == ENV_R) {
            SDL_SetRenderDrawColor(g->renderer, 0,0,255,255);
        } else {
            SDL_SetRenderDrawColor(g->renderer, 0,0,0,255);
        }
        //SDL_RenderFillRect(g->renderer, &indicator);
        SDL_RenderFillRect(g->renderer, &this->get_rect());

    }
};

#define NUM_SEQ_DIVISIONS 64

struct Sequencer: Block {
    float value[NUM_SEQ_DIVISIONS];
    uint32_t samples_per_division;
    uint32_t sample_num;

    Sequencer(int samples_per_division) {
        memset(this->value, 0, 64);
        this->samples_per_division = samples_per_division;
        this->sample_num = 0;
    }

    void set(int index) {
        this->value[index] = 1;
    }

    void unset(int index) {
        this->value[index] = 0;
    }
    void grab_next_impl(float *buf) {
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
};

typedef struct {
    int n_samples;
    float amplitude;
    ringbuf buf;
} echo;

struct Dummy: Block {
    float data[FRAMES_PER_BUFFER];
    Dummy(float *data) {
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            this->data[i] = data[i];
        }
    }
    void grab_next_impl(float *buf) {
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            *buf++ = data[i];
        }
    }
};

#endif
