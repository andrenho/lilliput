#include "video.h"

#include <stdio.h>
#include <syslog.h>

#include <SDL2/SDL.h>

#define WIDTH  348
#define HEIGHT 261
#define BORDER  10


static int              zoom   = 1;
static SDL_Window*      window = NULL;
static SDL_Renderer*    ren = NULL;
static bool             active = true;

//
// PALETTE
//

typedef struct { uint8_t r, g, b; } Color;

// this palette is based on the one defined in <http://androidarts.com/palette/16pal.htm>
static Color palette[256] = {
    {   0,   0,   0 },
    { 157, 157, 157 },
    { 255, 255, 255 },
    { 190,  38,  51 },
    { 224, 111, 139 },
    {  73,  60,  43 },
    { 164, 100,  34 },
    { 235, 137,  49 },
    { 247, 226, 107 },
    {  47,  72,  78 },
    {  68, 137,  26 },
    { 163, 206,  39 },
    {  27,  38,  50 },
    {   0,  87, 132 },
    {  49, 162, 242 },
    { 178, 220, 239 },
};

// 
// CONSTRUCTOR / DESTRUCTOR
//

void video_init(Config* config)
{
    zoom = config->zoom;

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        syslog(LOG_ERR, "Unable to initialize SDL: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("lilliput", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            (WIDTH + BORDER*2) * zoom, (HEIGHT + BORDER*2) * zoom, 0);
    if(!window) {
        syslog(LOG_ERR, "Could not create window: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
    if(!ren) {
        syslog(LOG_ERR, "Could not create renderer: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    syslog(LOG_DEBUG, "Video initialized.");

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 0xFF);
    SDL_RenderClear(ren);
}


void video_destroy()
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(window);
    SDL_Quit();
    syslog(LOG_DEBUG, "Video destroyed.");
}



// 
// HOST COMMANDS
//

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
    SDL_RenderPresent(ren);
}


// 
// GUEST COMMANDS
//

void video_clrscr(uint8_t idx)
{
    SDL_SetRenderDrawColor(ren, palette[idx].r, palette[idx].g, palette[idx].b, 0xFF);
    SDL_RenderDrawRect(ren, &(SDL_Rect) { 
        BORDER * zoom, BORDER * zoom, WIDTH * zoom, HEIGHT * zoom 
    });
}


void video_setbordercolor(uint8_t idx)
{
    SDL_SetRenderDrawColor(ren, palette[idx].r, palette[idx].g, palette[idx].b, 0xFF);
    const SDL_Rect rects[4] = {
        { 0, 0, (WIDTH + BORDER*2) * zoom, BORDER * zoom },                         // top
        { 0, (BORDER + HEIGHT) * zoom, (WIDTH + BORDER*2) * zoom, BORDER * zoom },  // bottom
        { 0, BORDER * zoom, BORDER * zoom, HEIGHT * zoom },                         // left
        { (BORDER + WIDTH) * zoom, BORDER * zoom, BORDER * zoom, HEIGHT * zoom },   // right
    };
    SDL_RenderFillRects(ren, rects, 4);
}


void video_drawpoint(uint16_t x, uint16_t y, uint8_t idx)
{
    SDL_SetRenderDrawColor(ren, palette[idx].r, palette[idx].g, palette[idx].b, 0xFF);
    SDL_RenderFillRect(ren, &(SDL_Rect) { 
        (x + BORDER) * zoom, (y + BORDER) * zoom, zoom, zoom
    });
}
