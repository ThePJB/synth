#ifndef BLOCK_H
#define BLOCK_H

#include "wavetable.h"

#define MAX_PARENTS 2
#define FRAMES_PER_BUFFER 256   

typedef enum {
    ENV_A,
    ENV_D,
    ENV_S,
    ENV_R,
    ENV_OFF,
} envelope_state;

typedef struct {
    envelope_state state;
    int sample_counter;
    float previous;

    int attack; // num samples
    int decay; // num samples
    float sustain; // sustain volume
    int release; // num samples

    float ref;  // last point we are coming down from
    float previous_out;
} envelope;

typedef union {
    wavetable *wt;
    double constant;
    envelope env;
} block_data;

typedef enum {
    BLOCK_WAVETABLE,
    BLOCK_GATE,
    BLOCK_TRIGGER,
    BLOCK_CONSTANT,
    BLOCK_ENVELOPE,
    BLOCK_MIXER,
} block_type;

typedef struct block {
    struct block *parents[MAX_PARENTS];
    block_data data;
    block_type type;
} block;

void grab(block *b, float *buf);

#endif
