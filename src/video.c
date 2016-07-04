#include "video.h"

#include <stdio.h>
#include <syslog.h>

#include <SDL2/SDL.h>

#define WIDTH  348
#define HEIGHT 261
#define BORDER  10


static int         zoom;
static SDL_Window* window;


void video_init(Config* config)
{
    zoom = config->zoom;

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        syslog(LOG_ERR, "Unable to initialize SDL: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("lillipad", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            WIDTH * zoom, HEIGHT * zoom, 0);
}


void video_destroy()
{
    SDL_Quit();
}
