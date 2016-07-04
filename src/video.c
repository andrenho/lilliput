#include "video.h"

#include <stdio.h>
#include <syslog.h>

#include <SDL2/SDL.h>

#define WIDTH  348
#define HEIGHT 261
#define BORDER  10


static int         zoom   = 1;
static SDL_Window* window = NULL;
static bool        active = true;


void video_init(Config* config)
{
    zoom = config->zoom;

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        syslog(LOG_ERR, "Unable to initialize SDL: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("lillipad", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            WIDTH * zoom, HEIGHT * zoom, 0);
    if(!window) {
        syslog(LOG_ERR, "Could not create window: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}


void video_destroy()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}


bool video_active()
{
    return active;
}


void video_doevents()
{
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_QUIT:
                active = false;
                break;
        }
    }
}


void video_draw()
{
}


