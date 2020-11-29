#!/usr/bin/sh

LDFLAGS="-lSDL2 -lSDL2_image -lrt -lasound -ljack -lpthread -lportaudio -lm "

INCLUDES="-I/usr/include/SDL2 -Iinc/"

CFLAGS="-Wall -Werror -Wno-unused-variable -Wno-unused-const-variable -Wno-unused-but-set-variable -g"

gcc test_seq.c block.c wavetable.c util.c -o test_seq $CFLAGS $INCLUDES $LDFLAGS
./test_seq