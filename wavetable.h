#ifndef WAVETABLE_H
#define WAVETABLE_H

typedef struct {
    float *data;
    int len;
    int current;
    int steps;
} wavetable;


float wt_sample(wavetable *wt);
void wt_walk(wavetable *wt);
wavetable wt_create(int len, int steps);

#endif
