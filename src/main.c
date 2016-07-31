#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "luisavm.h"
#include <SDL2/SDL.h>

#define WIDTH  318
#define HEIGHT 234
#define BORDER  10
#define ZOOM     3

static SDL_Renderer* ren = NULL;

//
// CALLBACKS
//

static void setpal(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
}


static void clrscr(uint8_t color)
{
    //SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(ren, &(SDL_Rect) { BORDER * ZOOM, BORDER * ZOOM, WIDTH * ZOOM, HEIGHT * ZOOM });
}

// 
// MAIN
//

static bool get_events(LVM_Computer* comp)
{
    (void) comp;

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_QUIT:
                return false;
        }
    }
    return true;
}


int main()
{
    // 
    // initialization
    //

    LVM_Computer* computer = lvm_computercreate(128 * 1024);

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window* window = SDL_CreateWindow("luisavm " VERSION, 
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            (WIDTH + (2*BORDER)) * ZOOM, (HEIGHT + (2 * BORDER)) * ZOOM, 0);
    if(!window) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
    if(!ren) {
        fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 0xFF);
    SDL_RenderClear(ren);
    SDL_RenderPresent(ren);

    //
    // setup video device
    //
#define CB(name) .name = name
    lvm_setupvideo(computer, (VideoCallbacks) {
        CB(setpal),
        CB(clrscr),
    });
#undef CB

    // 
    // main loop
    //

    Uint32 last_frame = SDL_GetTicks();
    while(1) {
        lvm_step(computer, 0);
        if(SDL_GetTicks() >= last_frame + 16) {
            last_frame = SDL_GetTicks();
            SDL_RenderPresent(ren);
            if(get_events(computer) == 0) {
                break;
            }
        }
    }

    // 
    // finalization
    //
    
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(window);
    SDL_Quit();

    lvm_computerdestroy(computer);
}
