#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "luisavm.h"
#include <SDL2/SDL.h>


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
            984, 732, 0);
    if(!window) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Renderer* ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
    if(!ren) {
        fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 0xFF);
    SDL_RenderClear(ren);
    SDL_RenderPresent(ren);

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
