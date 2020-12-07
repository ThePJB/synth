#!/usr/bin/sh

LDFLAGS="-lSDL2 -lSDL2_image -lrt -lasound -ljack -lpthread -lportaudio -lm "

INCLUDES="-I/usr/include/SDL2 -Iinc/"

CFLAGS="-Wall -Werror -Wno-unused-variable -Wno-unused-const-variable -Wno-address-of-temporary -g"

clang++ test_env.cpp block.cpp wavetable.cpp util.cpp -o test_env $CFLAGS $INCLUDES $LDFLAGS
./test_env