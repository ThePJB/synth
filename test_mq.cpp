#include "message_queue.h"
#include <stdio.h>

int main(int argc, char** argv) {
    auto mq = MessageQueue(sizeof(int), 3);
    int buf = 0;

    printf("4\n");
    int i = 4; mq.push(&i);
    printf("10\n");
    int j = 10; mq.push(&j);
    printf("69\n");
    int k = 69; mq.push(&k);
    while (mq.has_more()) {
        mq.pop(&buf);
        printf("popped %d\n", buf);
    }
    printf("should be empty\n");
    mq.push(&i);
    mq.push(&j);
    mq.push(&k);
    printf("pop 1\n");
    printf("start: %d end: %d lp? %d\n", mq.start, mq.end, mq.last_push);
    mq.pop(&buf);
    printf("push 1\n");
    printf("start: %d end: %d lp? %d\n", mq.start, mq.end, mq.last_push);
    mq.push(&i);
    printf("pop 1\n");
    printf("start: %d end: %d lp? %d\n", mq.start, mq.end, mq.last_push);
    mq.pop(&buf);
    printf("push 1\n");
    printf("start: %d end: %d lp? %d\n", mq.start, mq.end, mq.last_push);
    mq.push(&i);
    printf("pop 1\n");
    printf("start: %d end: %d lp? %d\n", mq.start, mq.end, mq.last_push);
    mq.pop(&buf);
    printf("push 1\n");
    printf("start: %d end: %d lp? %d\n", mq.start, mq.end, mq.last_push);
    mq.push(&i);
    printf("should die now\n");
    printf("start: %d end: %d lp? %d\n", mq.start, mq.end, mq.last_push);
    mq.push(&i);
    printf("shouldnt print this\n");
    printf("start: %d end: %d lp? %d\n", mq.start, mq.end, mq.last_push);
    mq.push(&i);
    printf("shouldnt print this\n");
    printf("start: %d end: %d lp? %d\n", mq.start, mq.end, mq.last_push);
    mq.push(&i);
    printf("shouldnt print this\n");
    printf("start: %d end: %d lp? %d\n", mq.start, mq.end, mq.last_push);
    mq.push(&i);
    printf("shouldnt print this\n");
    printf("start: %d end: %d lp? %d\n", mq.start, mq.end, mq.last_push);
    mq.push(&i);
    printf("shouldnt print this\n");
    printf("start: %d end: %d lp? %d\n", mq.start, mq.end, mq.last_push);
}