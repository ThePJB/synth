#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "block.h"
#include "graphics.h"

#define MAX_BLOCKS 100

struct Workspace {
    Block *block_ptrs[MAX_BLOCKS] = {0};
    int num_blocks = 0;

    Workspace() {};

    void draw() {
        for (int i = 0; i < num_blocks; i++) {
            if (!this->block_ptrs[i]) {
                printf("warning, bad block %d\n", i);
            }
            this->block_ptrs[i]->draw();
        }
    }

    void connect(Block *parent, Block* child) {
        if (!parent) printf("null parent");
        if (!child) printf("null child");

        parent->child = child;
        for (int i = 0; i < MAX_PARENTS; i++) {
            if (child->parents[i] == 0) {
                child->parents[i] = parent;
            }
            break;
        }
    }

    void disconnect(Block *parent, Block* child) {
        if (!parent) printf("null parent");
        if (!child) printf("null child");

        parent->child = 0;
        for (int i = 0; i < MAX_PARENTS; i++) {
                if (child->parents[i] == parent) {
                    child->parents[i] = 0;
                    break;
                }
            }
    }
    void register_block(Block *b) {
        this->block_ptrs[num_blocks++] = b;
    }

    void delete_block(Block *block) {
        // find index
        int index = 0;
        while (this->block_ptrs[index++] != block);

        // remove connections
        for (int i = 0; i < MAX_PARENTS; i++) {
            if (block->parents[i] != 0) {
                this->disconnect(block->parents[i], block);
            }
        }
        if (block->child != 0) {
            this->disconnect(block, block->child);
        }

        // free memory
        free(this->block_ptrs[index]);

        // update ws
        this->block_ptrs[index] = this->block_ptrs[this->num_blocks--];
    }


};

#endif