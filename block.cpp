#include "block.h"

void BlockMessage::print() {
    if (this->type == FAIL_GRAB) {
        if (this->parent == NULL && this->child == NULL) {
            printf("Failed to grab from <null parent> to <null child>\n");
        } else if (this->parent == NULL && this->child != NULL) {
            printf("Failed to grab from <null parent> to %s\n", child->name);
        } else if (this->parent != NULL && this->child == NULL) {
            printf("Failed to grab from %s to <null child> and also how did this happen?\n", parent->name);
        } else {
            printf("Failed to grab from %s to %s\n", parent->name, child->name);
        }

        // probably segfault because null parent right?
        //printf("Failed to grab from %s to %s\n", parent->name, child->name);

    } else if (this->type == TEST) {
        printf("Test from %s\n", child->name);
    }
}