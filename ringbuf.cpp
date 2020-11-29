#include <stdlib.h>
#include "ringbuf.h"

ringbuf make_ringbuf(uint32_t len) {
    ringbuf ret = {0};
    ret.data = calloc(len, sizeof(float));
    ret.len = len;
    ret.start = 0;
    ret.end = 0;
    return ret;
}

void ringbuf_push_back(ringbuf *buf, float val) {
    buf->data[buf->end] = val;
    buf->end = buf->end + 1 % buf->len;
}

float ringbuf_pop_front(ringbuf *buf) {
    float answer = buf->data[buf->start];
    buf->start = buf->start + 1 % buf->len;
    return answer;
}
