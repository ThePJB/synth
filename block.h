#ifndef BLOCK_H
#define BLOCK_H

#include "wavetable.h"
#include "ringbuf.h"
#include "string.h"
#include "graphics.h"
#include <stdint.h>
#include <SDL.h>


#define MAX_PARENTS 4
#define FRAMES_PER_BUFFER 256

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

    SDL_Rect get_rect() {
        return SDL_Rect{this->x, this->y, this->w, this->h};
    }

    virtual void grab(float *buf) = 0;
    void draw() {
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
            b->grab(buf);
        } else {
            memset(buf, 0, FRAMES_PER_BUFFER);
        }
    }


};

struct Wavetable: Block {
    wavetable *wt;
    Wavetable(wavetable *wt) { this->wt = wt; }
    void grab(float *buf);
};

struct Gate: Block {
    Gate() {};
    void grab(float *buf);
};

struct Trigger: Block {
    float state;

    Trigger() {
        state = 0;
    }
    virtual void draw() {
        Block::draw();
        auto g = Graphics::get();
        SDL_Rect indicator = SDL_Rect{this->x + 3, this->y + 3, 3, 3};
        if (state == 1) {
            SDL_SetRenderDrawColor(g->renderer, 255,255,255,255);
        } else {
            SDL_SetRenderDrawColor(g->renderer, 0,0,0,255);
        }
        SDL_RenderFillRect(g->renderer, &indicator);

    }

    void set_trigger() { this->state = 1; }
    void unset_trigger() { this->state = 0; }
    void grab(float *buf);
};

struct Mixer: Block {
    float gain;

    Mixer(float gain) {
        this->gain = gain;
    }
    void grab(float *buf);
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
    void grab(float *buf);
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
    void grab(float *buf);
};

typedef struct {
    int n_samples;
    float amplitude;
    ringbuf buf;
} echo;

#endif
