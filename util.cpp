#include "util.h"

uint64_t get_us() { 
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000 + (uint64_t)t.tv_nsec / 1000;
}


float lerp(float a, float b, float x) {
    return a*(1-x) + b*x;
}

float unlerp(float a, float b, float x) {
    return (x - a) / (b - a);
}

float slow_start2(float x) {
    return x*x;
}

float slow_start3(float x) {
    return x*x*x;
}

float slow_stop2(float x) {
    return 1.0 - slow_start2(1.0 - x);
}

float slow_stop3(float x) {
    return 1.0 - slow_start3(1.0 - x);
}

float remap(float prev_lower, float prev_upper, float new_lower, float new_upper, float x) {
    return lerp(new_lower, new_upper, unlerp(prev_lower, prev_upper, x));
}