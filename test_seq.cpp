#include <stdio.h>
#include "block.h"

int main(int argc, char** argv) {
    block tseq = {
        .type = BLOCK_SEQUENCER,
        .data.seq = (sequencer) {
            .samples_per_division = 1,
            .sample_num = 0,
            .value = {
                1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            },
        },
    };

    float buf[256] = {0};

    printf("a\n");
    grab(&tseq, buf);
    printf("b\n");

    for (int i = 0; i < 64; i++) {
        printf("%d: %f\n", i, buf[i]);
    }
    printf("done\n");
}