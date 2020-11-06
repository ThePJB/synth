#!/usr/bin/sh

LDFLAGS="-lSDL2 -lSDL2_image -lrt -lasound -ljack -lpthread -lportaudio -lm "

INCLUDES="-I/usr/include/SDL2 -Iinc/"

CFLAGS="-Wall -Werror -Wno-unused-variable -Wno-unused-const-variable -Wno-unused-but-set-variable -g"

gcc main.c block.c wavetable.c util.c -o synth $CFLAGS $INCLUDES $LDFLAGS
