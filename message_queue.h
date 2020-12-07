
#pragma once

struct Message {
    virtual void print() = 0;
};

struct MessageQueue;
static MessageQueue *mq_instance;

struct MessageQueue {
    enum msg_type {
        TRY_GRAB_FAIL,
    };

    MessageQueue() {
        this->messages = (Message *)malloc(sizeof(Message) * this->size);
    };

    Message *messages;
    int size = 1024;
    int start = 0;
    int end = 0;

    static void init() {
        printf("Initializing message queue...");
        mq_instance = new MessageQueue();
    }

    static MessageQueue *get() {
        return mq_instance;
    }

    void push(Message m) {
        this->messages[this->end++] = m;
        if (this->end >= this->size) this->end = 0;
    }

    Message pop() {
        Message m = this->messages[this->start++];
        if (this->start >= this->size) this->start = 0;
        return m;
    }

    bool has_more() {
        return this->start < this->end;
    }
};