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
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Texture *atlas;
    int xres;
    int yres;
    int zoom_factor;
} graphics;

typedef struct {
   PaStream *stream;
   PaError err; 
} audio;


void teardown();
void print_pa_err(PaError err);
graphics gg;
audio ag;

wavetable wt_sine;
wavetable wt_saw;
wavetable wt_square;
wavetable wt_triangle;
wavetable wt_noise;

wavetable *wavetables[] = {&wt_sine, &wt_saw, &wt_square, &wt_triangle, &wt_noise};

graphics graphics_init(int xres, int yres, int zoom) {
    graphics g = {0};

    /* setup */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) die("couldn't init sdl");

    g.window = SDL_CreateWindow("Best Twin Stick Shooter", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        xres, yres, SDL_WINDOW_SHOWN);
    if (g.window == NULL) die("couldn't create window");

    g.renderer = SDL_CreateRenderer(g.window, -1, SDL_RENDERER_ACCELERATED);
    if (g.renderer == NULL) die("couldn't create renderer");

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) die("couldn't initialize image loading");

    /* load assets */
    /*
    SDL_Surface* loaded_surface = IMG_Load("assets/atlas.png");
    g.atlas = SDL_CreateTextureFromSurface(g.renderer, loaded_surface);
    if (g.atlas == NULL) die("couldn't create texture");
    SDL_FreeSurface(loaded_surface);
    */

    g.xres = xres;
    g.yres = yres;
    g.zoom_factor = zoom;

    return g;
}


#define xres 640
#define yres 480


static int test_callback(
        const void *inputBuffer,
        void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData) {

    stereo *data = (stereo*)userData;  // whats userdata?
    float *out = (float*)outputBuffer;
    
    data->out_block->grab(out); 
    
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

    auto q_osc = Wavetable(&wt_sine);
    
    auto q_trig = Trigger();
    auto q_env = Envelope(3000, 3000, 0.5, 4800);
    q_env.set_parent(0, &q_trig);

    auto q_gate = Gate();
    q_gate.set_parent(0, &q_osc);
    q_gate.set_parent(1, &q_env);

    auto lfo = Wavetable(&wt_lfo);
    auto lfo_gate = Gate();
    lfo_gate.set_parent(0, &q_osc);
    lfo_gate.set_parent(1, &lfo);


    auto w_osc = Wavetable(&wt_noise);
    auto w_seq = Sequencer(SAMPLE_RATE * 3 / NUM_SEQ_DIVISIONS);
    w_seq.set(0);
    w_seq.set(16);
    w_seq.set(32);
    w_seq.set(48);
    w_seq.set(56);
    
    auto w_env = Envelope(1000, 20000, 0, 3000);
    auto w_gate = Gate();

    w_gate.set_parent(0, &w_env);
    w_gate.set_parent(1, &w_osc);

    w_env.set_parent(0, &w_seq);


    
    auto mixer = Mixer(0.5);
    mixer.set_parent(0, &q_gate);
    mixer.set_parent(1, &w_gate);
    mixer.set_parent(2, &q_gate);

    stereo data;
    data.l = 0;
    data.r = 0;
    data.amplitude = 0.5;
    data.steps = 1;
    data.out_block = &mixer;
    //data.current_wavetable = wavetables[0];

    gg = graphics_init(xres, yres, 1); 
    ag = audio_init(&data);

    int frame_period = 1000000/180;

    SDL_Event e;
    while (true) {
        int mouse_x = 0;
        int mouse_y = 0;

        uint64_t frame_start_us = get_us();
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) teardown();
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        teardown();
                        break;
                    case SDLK_q:
                        q_trig.set_trigger();
                        break;
                    case SDLK_w:
                        //w_trig.set_trigger();
                        break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_q:
                        q_trig.unset_trigger();
                        break;
                    case SDLK_w:
                        //w_trig.unset_trigger();
                        break;
                }
            }
        }

        // clicc
        if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        }

        /* draw */
        SDL_SetRenderDrawColor(gg.renderer, 0,0,0,255);
        SDL_RenderClear(gg.renderer);


        SDL_RenderPresent(gg.renderer);

        uint64_t frame_end_us = get_us();
        uint64_t delta_us = frame_end_us - frame_start_us;
        if (delta_us < frame_period) {
            usleep(frame_period - delta_us);
        }
        //printf("frame time %f ms\n", delta_us / 1000.0);
    }
}

void teardown_graphics() {
    SDL_DestroyRenderer(gg.renderer);
    SDL_DestroyWindow(gg.window);
    IMG_Quit();
    SDL_Quit();
}

void print_pa_err(PaError err) {
    fprintf(stderr, "Killed by error (%d) %s\n", err, Pa_GetErrorText(err));
    teardown();
}

void teardown() {
    ag.err = Pa_StopStream(ag.stream); if (ag.err != paNoError) print_pa_err(ag.err);
    ag.err = Pa_CloseStream(ag.stream); if (ag.err != paNoError) print_pa_err(ag.err);
    Pa_Terminate();
    teardown_graphics();
    exit(0);
}
