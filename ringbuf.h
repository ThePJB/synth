#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdint.h>

typedef struct {
    uint32_t len;
    uint32_t start;
    uint32_t end;
    float *data;
} ringbuf;

ringbuf make_ringbuf(uint32_t size);
void ringbuf_push_back(ringbuf *buf, float val);
float ringbuf_pop_front(ringbuf *buf);

#endif
