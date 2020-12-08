
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct MessageQueue {

    uint8_t *messages;
    size_t elemsize;
    size_t len;
    size_t start = 0;
    size_t end = 0;
    bool last_push;

    MessageQueue(size_t elemsize, size_t len) {
        this->messages = (uint8_t*)malloc(this->elemsize * this->len);
        this->len = len;
        this->elemsize = elemsize;
    };

    void push(const void *buf) {
        if (this->full()) {
            printf("tried pushing to full queue\n");
            exit(1);
        }
        memcpy(this->messages + this->end, buf, this->elemsize);
        end += this->elemsize;
        if (end >= this->len * this->elemsize) {
            end = 0;
        }
        this->last_push = true;
    }

    void pop(void *buf) {
        if (!this->has_more()) {
            printf("tried popping from empty queue\n");
            exit(1);
        }
        memcpy(buf, this->messages + this->start, this->elemsize);
        start += this->elemsize;
        if (start >= this->len * this->elemsize) {
            start = 0;
        }
        this->last_push = false;
    }

    bool has_more() {
        return !this->empty();
    }

    bool empty() {
        return this->start == this->end && !this->last_push;
    }

    bool full() {
        return this->start == this->end && this->last_push;
    }
};