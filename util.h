#ifndef UTIL_H
#define UTIL_H

#include <time.h>
#include <stdint.h>

#define true 1
#define false 0

#define LENGTH(X) (sizeof(X) / sizeof(X[0]))

uint64_t get_us();

// easing functions
float lerp(float a, float b, float x);
float unlerp(float a, float b, float x);
float remap(float prev_lower, float prev_upper, float new_lower, float new_upper, float x);
float slow_start2(float x);
float slow_start3(float x);
float slow_stop2(float x);
float slow_stop3(float x);


#endif
