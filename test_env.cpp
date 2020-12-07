#include "block.h"
#include "workspace.h"

int main(int argc, char** argv) {
    Workspace ws = Workspace();
    Envelope *q_env = new Envelope(10, 10, 0.5, 10); ws.register_block(q_env);
    float dummydata[FRAMES_PER_BUFFER] = {0};
    for (int i = 0; i < 100; i++) {
        dummydata[100 + i] = 1;
    }
    Dummy *dummy = new Dummy(dummydata);
    ws.connect(dummy, q_env);

    float out[FRAMES_PER_BUFFER] = {0};
    q_env->grab(out);
    for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
        printf("%.1f ", out[i]);
    }
    
}