#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>
#include <math.h>

#include "portaudio.h"
#include "util.h"
#include "wavetable.h"
#include "block.h"
#include "graphics.h"
#include "workspace.h"

#define SAMPLE_RATE (48000)

#define die(X) printf("%s %d %s: dying -- %s\n", __FILE__, __LINE__, __func__, X), teardown()

typedef struct {
    float l;
    float r;

    float amplitude;
    int steps;
    //wavetable *current_wavetable;
    Block *out_block;
} stereo;



typedef struct {
   PaStream *stream;
   PaError err; 
} audio;


void print_pa_err(PaError err);
void teardown();

audio ag;

wavetable wt_sine;
wavetable wt_saw;
wavetable wt_square;
wavetable wt_triangle;
wavetable wt_noise;

wavetable *wavetables[] = {&wt_sine, &wt_saw, &wt_square, &wt_triangle, &wt_noise};


#define xres 1920
#define yres 1080

int audio_warning = 0;

static int test_callback(
        const void *inputBuffer,
        void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData) {

    stereo *data = (stereo*)userData;  // whats userdata?
    float *out = (float*)outputBuffer;
    
    if (data->out_block) {
        //data->out_block->grab(out); 
        audio_warning = 0;
    } else {
        audio_warning = 1;
    }
    
    return 0;
}

audio audio_init(void *data) {
   audio a = {0};

    a.err = Pa_Initialize(); if (a.err != paNoError) print_pa_err(a.err);
    a.err = Pa_OpenDefaultStream(
            &a.stream,
            0, // input channels
            1, // output channels
            paFloat32,
            SAMPLE_RATE,
            FRAMES_PER_BUFFER, // frames per buffer ??
            test_callback,
            data); 
    if (a.err != paNoError) teardown();
    a.err = Pa_StartStream(a.stream); if (a.err != paNoError) print_pa_err(a.err);

    return a;
}

bool rect_test(int x, int y, SDL_Rect r) {
    return x > r.x && x < r.x + r.w &&
            y > r.y && y < r.y + r.h;
}

int main(int argc, char** argv) { 
    
    int wt_len = 48000;
    int wt_sel = 0;

    wt_sine = wt_create(wt_len, 200);
    for (int i = 0; i < wt_len; i++) {
        wt_sine.data[i] = (float)sin((double)i/(double)wt_len * 2.0 * M_PI);
    }
    wavetable wt_lfo = wt_create(wt_len, 1);
    for (int i = 0; i < wt_len; i++) {
        wt_lfo.data[i] = (float)sin((double)i/(double)wt_len * 2.0 * M_PI);
    }
    wt_saw = wt_create(wt_len, 200);
    for (int i = 0; i < wt_len; i++) {
        wt_saw.data[i] = (float)((double)i/(double)(wt_len));
    }
    wt_square = wt_create(wt_len, 40);
    for (int i = 0; i < wt_len; i++) {
        if (i < wt_len/2) {
            wt_square.data[i] = 1;
        } else {
            wt_square.data[i] = 0;
        }
    }
    wt_triangle = wt_create(wt_len, 100);
    for (int i = 0; i < wt_len; i++) {
        if (i < wt_len/2) {
            wt_triangle.data[i] = 2.0 * (float)((double)i/(double)(wt_len));
        } else {
            wt_triangle.data[i] = 2.0 * (float)(1.0 - (double)i/(double)(wt_len));
        }
    }
    wt_noise = wt_create(wt_len, 1);
    for (int i = 0; i < wt_len; i++) {
        wt_noise.data[i] = (float)rand() / (float)RAND_MAX;
    }

    Workspace ws = Workspace();
    Wavetable *q_osc = new Wavetable(&wt_sine); ws.register_block(q_osc);
    Trigger *q_trig = new Trigger();  ws.register_block(q_trig);
    Envelope *q_env = new Envelope(3000, 3000, 0.5, 4800); ws.register_block(q_env);
    ws.connect(q_trig, q_env);
    
    Gate *q_gate = new Gate(); ws.register_block(q_gate);
    ws.connect(q_osc, q_gate);
    ws.connect(q_env, q_gate);
    
/*
    MKBLOCK(w_osc) Wavetable(&wt_noise);
    MKBLOCK(w_seq) Sequencer(SAMPLE_RATE * 3 / NUM_SEQ_DIVISIONS);
    ((Sequencer*)w_seq)->set(0); // wat do about this shit
    ((Sequencer*)w_seq)->set(16);
    ((Sequencer*)w_seq)->set(32);
    ((Sequencer*)w_seq)->set(48);
    ((Sequencer*)w_seq)->set(56);
    
    MKBLOCK(w_env) Envelope(1000, 20000, 0, 3000);
    MKBLOCK(w_gate) Gate();

    ws.connect(w_seq, w_env);
    ws.connect(w_env, w_gate);
    ws.connect(w_osc, w_gate);
    */
    Mixer *mixer = new Mixer(0.5); ws.register_block(mixer);
    ws.connect(q_gate, mixer);
    //ws.connect(w_gate, mixer);

    printf("mixer %p\n", mixer);

    stereo data;
    data.l = 0;
    data.r = 0;
    data.amplitude = 0.5;
    data.steps = 1;
    data.out_block = mixer;
    //data.current_wavetable = wavetables[0];

    Graphics::init(xres, yres, "mod synth");

    ag = audio_init(&data);

    int frame_period = 1000000/180;

    SDL_Event e;

    Block *rollover_block;
    int last_click_x;
    int last_click_y;

    while (true) {
        int mouse_x = 0;
        int mouse_y = 0;

        uint64_t frame_start_us = get_us();
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) teardown();
            if (e.type == SDL_MOUSEMOTION) {
                if (e.motion.state & SDL_BUTTON_LMASK) {
                    // dragging
                    if (!rollover_block) continue;
                    rollover_block->x += e.motion.xrel;
                    rollover_block->y += e.motion.yrel;
                } else {
                    // rollover
                    bool any_rollover = 0;
                    for (int i = 0; i < ws.num_blocks; i++) {
                        if (rect_test(e.motion.x, e.motion.y, ws.block_ptrs[i]->get_rect())) {
                            rollover_block = ws.block_ptrs[i];
                            any_rollover = 1;
                            break;
                        }
                    }
                    if (!any_rollover) rollover_block = 0;
                }
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        teardown();
                        break;
                    case SDLK_q:
                        ((Trigger*)q_trig)->set_trigger();
                        break;
                    case SDLK_w:
                        //w_trig.set_trigger();
                        break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_q:
                        ((Trigger*)q_trig)->unset_trigger();
                        break;
                    case SDLK_w:
                        //w_trig.unset_trigger();
                        break;
                }
            }
        }

        if (audio_warning) {
            printf("warning: no out block");
        }

        // clicc
        if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        }

        /* draw */
        auto g = Graphics::get();
        SDL_SetRenderDrawColor(g->renderer, 0,0,0,255);
        SDL_RenderClear(g->renderer);

        ws.draw();

        SDL_RenderPresent(g->renderer);

        uint64_t frame_end_us = get_us();
        uint64_t delta_us = frame_end_us - frame_start_us;
        if (delta_us < frame_period) {
            usleep(frame_period - delta_us);
        }
        //printf("frame time %f ms\n", delta_us / 1000.0);
    }
}

void print_pa_err(PaError err) {
    fprintf(stderr, "Killed by error (%d) %s\n", err, Pa_GetErrorText(err));
    teardown();
}

void teardown() {
    ag.err = Pa_StopStream(ag.stream); if (ag.err != paNoError) print_pa_err(ag.err);
    ag.err = Pa_CloseStream(ag.stream); if (ag.err != paNoError) print_pa_err(ag.err);
    Pa_Terminate();
    Graphics::teardown();
    exit(0);
}
