#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "block.h"
#include "graphics.h"

#define MAX_BLOCKS 100

struct Workspace {
    Block *block_ptrs[MAX_BLOCKS];
    int num_blocks = 0;

    Workspace() {};

    void draw() {
        for (int i = 0; i < MAX_BLOCKS) {
            this->blocks[i]->draw();
        }
    }

    void connect(Block *parent, Block* child) {
        parent->child = child;
        for (int i = 0; i < MAX_PARENTS; i++) {
            if (child->parents[i] == 0) {
                child->parents[i] = parent;
            }
            break;
        }
    }

    void disconnect(Block *parent, Block* child) {
        parent->child = 0;
        for (int i = 0; i < MAX_PARENTS; i++) {
                if (child->parents[i] == parent) {
                    child->parents[i] = 0;
                    break;
                }
            }
    }

    Block *alloc_block() {
        Block *new_block = (Block*)calloc(sizeof(Block), 1);
        this->block_ptrs[num_blocks++] = new_block;
        return new_block;
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