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
    block *out_block;
} stereo;

typedef struct { SDL_Renderer *renderer;
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

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG);

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
    
    grab(data->out_block, out); 
    
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
    if (a.err != paNoError) teardown(a.err);
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


    // Q Key
    block q_osc = {
        .type = BLOCK_WAVETABLE,
        .data.wt = &wt_saw,
    };
    block inq = {
        .type = BLOCK_TRIGGER,
        .data.constant = 0,
    };
    block qadsr = {
        .type = BLOCK_ENVELOPE,
        .data.env = {
            .attack = 3000,
            .decay = 3000,
            .sustain = 0.5,
            .release = 4800,
            .previous = 0,
            .sample_counter = 0,
            .state = ENV_OFF,
        }
    };
    qadsr.parents[0] = &inq;
    block qgate = {
        .type = BLOCK_GATE,
    };
    qgate.parents[0] = &q_osc;
    qgate.parents[1] = &qadsr;
    
    // W Seq
    block w_seq = {
        .type = BLOCK_SEQUENCER,
        .data.seq = (sequencer) {
            .samples_per_division = SAMPLE_RATE * 3 / NUM_SEQ_DIVISIONS,
            .sample_num = 0,
            .value = {
                1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            },
        },
    };
        


    // W Key
    block w_osc = {
        .type = BLOCK_WAVETABLE,
        .data.wt = &wt_noise,
    };
    block inw = {
        .type = BLOCK_TRIGGER,
        .data.constant = 0,
    };
    block wadsr = {
        .type = BLOCK_ENVELOPE,
        .data.env = {
            .attack = 1000,
            .decay = 20000,
            .sustain = 0.0,
            .release = 3000,
            .previous = 0,
            .sample_counter = 0,
            .state = ENV_OFF,
        }
    };
    //wadsr.parents[0] = &inw;
    wadsr.parents[0] = &w_seq;
    block wgate = {
        .type = BLOCK_GATE,
    };
    wgate.parents[0] = &w_osc;
    wgate.parents[1] = &wadsr;


    // E Key
    block kick_osc = {
        .type = BLOCK_WAVETABLE,
        .data.wt = &wt_square,
    };
    block kick_trigger = {
        .type = BLOCK_TRIGGER,
        .data.constant = 0,
    };
    block kick_adsr = {
        .type = BLOCK_ENVELOPE,
        .data.env = {
            .attack = 2400,
            .decay = 4800,
            .sustain = 0.0,
            .release = 4800,
            .previous = 0,
            .sample_counter = 0,
            .state = ENV_OFF,
        }
    };
    kick_adsr.parents[0] = &kick_trigger;
    block kick_gate = {
        .type = BLOCK_GATE,
    };
    kick_gate.parents[0] = &kick_osc;
    kick_gate.parents[1] = &kick_adsr;


    block mixer = {
        .type = BLOCK_MIXER,
    };
    mixer.parents[0] = &wgate;
    mixer.parents[1] = &kick_gate;
    mixer.parents[2] = &qgate;


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
                    case SDLK_h:
                        data.steps--;
                        printf("steps set to %d\n", data.steps);
                        break;
                    case SDLK_l:
                        data.steps++;
                        printf("steps set to %d\n", data.steps);
                        break;
                    case SDLK_j:
                        data.amplitude -= 0.1;  
                        break;
                    case SDLK_k:
                        data.amplitude += 0.1;  
                        break;
                    case SDLK_q:
                        inq.data.constant = 1.0;
                    break;
                    case SDLK_w:
                        inw.data.constant = 1.0;
                    break;
                    case SDLK_e:
                        kick_trigger.data.constant = 1.0;
                    break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_q:
                        inq.data.constant = 0.0;
                        break;
                    case SDLK_w:
                        inw.data.constant = 0.0;
                        break;
                    case SDLK_e:
                        kick_trigger.data.constant = 0.0;
                    break;
                }
            }
        }

        // clicc
        if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        }

        // print debug info for envelope
        if (wadsr.data.env.state == ENV_A) {
            printf("A\n");
        } else if (wadsr.data.env.state == ENV_D) {
            printf("D\n");
        } else if (wadsr.data.env.state == ENV_S) {
            printf("S\n");
        } else if (wadsr.data.env.state == ENV_R) {
            printf("R\n");
        } else if (wadsr.data.env.state == ENV_OFF) {
            printf("OFF\n");
        }
        printf("si: %d\n", w_seq.data.seq.sample_num / w_seq.data.seq.samples_per_division);
        printf("sn: %d\n", w_seq.data.seq.sample_num);
            


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
