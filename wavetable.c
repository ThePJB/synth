#include <stdlib.h>
#include "wavetable.h"

float wt_sample(wavetable *wt) {
    return wt->data[wt->current];
}

void wt_walk(wavetable *wt) {
    wt->current += wt->steps;
    if (wt->current > wt->len) wt->current -= wt->len;
}

wavetable wt_create(int len, int steps) {
    wavetable wt = {0};
    wt.data = malloc(len * sizeof(float)); 
    wt.len = len;
    wt.steps = steps;
   return wt;
}
