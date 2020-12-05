#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

#define die(X) printf("%s %d %s: dying -- %s\n", __FILE__, __LINE__, __func__, X), teardown()

struct Graphics;
static Graphics *graphics_instance;

struct Graphics {
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Texture *atlas;
    int xres;
    int yres;
    int zoom_factor;

    Graphics() {};

    static void init(int xres, int yres, char const *title) {
        printf("initializing graphics...\n");
        graphics_instance = new Graphics();
        
        //(Graphics*)malloc(sizeof(Graphics));

        /* setup */
        if (SDL_Init(SDL_INIT_VIDEO) < 0) die("couldn't init sdl");

        graphics_instance->window = SDL_CreateWindow(title, 
            SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED, 
            xres, yres, SDL_WINDOW_SHOWN);
        if (graphics_instance->window == NULL) die("couldn't create window");

        graphics_instance->renderer = SDL_CreateRenderer(graphics_instance->window, -1, SDL_RENDERER_ACCELERATED);
        if (graphics_instance->renderer == NULL) die("couldn't create renderer");

        if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) die("couldn't initialize image loading");

        /* load assets */
        /*
        SDL_Surface* loaded_surface = IMG_Load("assets/atlas.png");
        g.atlas = SDL_CreateTextureFromSurface(g.renderer, loaded_surface);
        if (g.atlas == NULL) die("couldn't create texture");
        SDL_FreeSurface(loaded_surface);
        */

        graphics_instance->xres = xres;
        graphics_instance->yres = yres;
        graphics_instance->zoom_factor = 1;
    }

    static void teardown() {
        SDL_DestroyRenderer(graphics_instance->renderer);
        SDL_DestroyWindow(graphics_instance->window);
        IMG_Quit();
        SDL_Quit();
    }

    static Graphics *get() {
        return graphics_instance;
    }
};

#endif