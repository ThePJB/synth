#ifndef BLOCK_H
#define BLOCK_H

#include "wavetable.h"
#include "ringbuf.h"
#include <stdint.h>

#define MAX_PARENTS 4
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
    uint32_t sample_counter;
    float previous;

    uint32_t attack; // num samples
    uint32_t decay; // num samples
    float sustain; // sustain volume
    uint32_t release; // num samples

    float ref;  // last point we are coming down from
    float previous_out;
} envelope;

#define NUM_SEQ_DIVISIONS 64

typedef struct {
    float value[NUM_SEQ_DIVISIONS];
    uint32_t samples_per_division;
    uint32_t sample_num;
} sequencer;

typedef struct {
    int n_samples;
    float amplitude;
    ringbuf buf;
} echo;

typedef union {
    wavetable *wt;
    double constant;
    envelope env;
    sequencer seq;
} block_data;

typedef enum {
    BLOCK_WAVETABLE,
    BLOCK_GATE,
    BLOCK_TRIGGER,
    BLOCK_CONSTANT,
    BLOCK_ENVELOPE,
    BLOCK_MIXER,
    BLOCK_SEQUENCER,
    BLOCK_ECHO,
} block_type;

typedef struct block {
    struct block *parents[MAX_PARENTS];
    block_data data;
    block_type type;
} block;

void grab(block *b, float *buf);

#endif
