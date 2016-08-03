#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "luisavm.h"
#include <SDL2/SDL.h>

#define WIDTH  318
#define HEIGHT 234
#define BORDER  20
#define ZOOM     2

static SDL_Window* window = NULL;
static SDL_Renderer* ren = NULL;
static SDL_Color pal[256];

typedef struct Sprites {
    uint32_t n_sprites;
    SDL_Texture** sprite;
} Sprites;
static Sprites sprites = { 0, NULL };


// {{{ CALLBACKS

static void setpal(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    pal[idx].r = r;
    pal[idx].g = g;
    pal[idx].b = b;
}


static void clrscr(uint8_t color)
{
    SDL_SetRenderDrawColor(ren, pal[color].r, pal[color].g, pal[color].b, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(ren, &(SDL_Rect) { BORDER * ZOOM, BORDER * ZOOM, WIDTH * ZOOM, HEIGHT * ZOOM });
}


static void change_border_color(uint8_t color)
{
    int w, h; SDL_GetWindowSize(window, &w, &h);
    SDL_SetRenderDrawColor(ren, pal[color].r, pal[color].g, pal[color].b, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(ren, &(SDL_Rect) { 0, 0, w, BORDER * ZOOM });
    SDL_RenderFillRect(ren, &(SDL_Rect) { 0, h - (BORDER * ZOOM), w, BORDER * ZOOM });
    SDL_RenderFillRect(ren, &(SDL_Rect) { 0, 0, BORDER * ZOOM, h });
    SDL_RenderFillRect(ren, &(SDL_Rect) { w - (BORDER * ZOOM), 0, BORDER * ZOOM, h });
}


static uint32_t upload_sprite(uint16_t w, uint16_t h, uint8_t* data)
{
    Uint32 rmask = 0xff000000;
    Uint32 gmask = 0x00ff0000;
    Uint32 bmask = 0x0000ff00;
    Uint32 amask = 0x000000ff;

    SDL_Surface* sf = SDL_CreateRGBSurface(0, w, h, 32, rmask, gmask, bmask, amask);
    SDL_SetColorKey(sf, SDL_TRUE, 0x00000000);
    for(size_t x=0; x<w; ++x) {
        for(size_t y=0; y<h; ++y) {
            uint8_t idx = data[x+(y*w)];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
            Uint32* target = (Uint32*)((Uint8*)sf->pixels + (y * (size_t)sf->pitch) + (x * 4));
#pragma GCC diagnostic pop
            if(idx != 0xFF) {
                *target = ((Uint32)pal[idx].r << 24) | ((Uint32)pal[idx].g << 16) | ((Uint32)pal[idx].b << 8) | 0xFF;
            } else {
                *target = 0x00000000;
            }
        }
    }

    SDL_Texture* tx = SDL_CreateTextureFromSurface(ren, sf);
    SDL_FreeSurface(sf);

    sprites.sprite = realloc(sprites.sprite, sizeof(SDL_Texture*) * (sprites.n_sprites + 1));
    sprites.sprite[sprites.n_sprites] = tx;
    return ++sprites.n_sprites;
}


static void draw_sprite(uint32_t sprite_idx, uint16_t pos_x, uint16_t pos_y)
{
    assert(sprite_idx > 0);
    assert(sprites.sprite[sprite_idx-1]);

    Uint32 format;
    int access, w, h;
    SDL_QueryTexture(sprites.sprite[sprite_idx-1], &format, &access, &w, &h);
    SDL_RenderCopy(ren, sprites.sprite[sprite_idx-1], NULL, 
            &(SDL_Rect) { (pos_x+BORDER) * ZOOM, (pos_y+BORDER) * ZOOM, w * ZOOM, h * ZOOM });
}

// }}}

// {{{ EVENTS

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

// }}}

// {{{ MAIN

int main()
{
    // 
    // initialization
    //

    LVM_Computer* computer = lvm_computercreate(128 * 1024, true);  // TODO

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("luisavm " VERSION, 
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
    // setup devices
    //
#define CB(name) .name = name
    lvm_setupvideo(computer, (VideoCallbacks) {
        CB(setpal),
        CB(clrscr),
        CB(change_border_color),
        CB(upload_sprite),
        CB(draw_sprite),
    });
#undef CB

    // 
    // main loop
    //

    Uint32 last_frame = SDL_GetTicks();
    while(1) {
        if(!lvm_debuggeractive(computer)) {
            lvm_step(computer, 0);
            if(SDL_GetTicks() >= last_frame + 16) {
                last_frame = SDL_GetTicks();
                SDL_RenderPresent(ren);
                if(get_events(computer) == 0) {
                    break;
                }
            }
        } else {
            lvm_debuggerupdate(computer);
            SDL_RenderPresent(ren);
            if(get_events(computer) == 0) {
                break;
            }
            SDL_Delay(1);
        }
    }

    // 
    // finalization
    //
    if(sprites.n_sprites > 0) {
        for(size_t i=0; i < sprites.n_sprites; ++i) {
            SDL_DestroyTexture(sprites.sprite[i]);
        }
        free(sprites.sprite);
    }
    
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(window);
    SDL_Quit();

    lvm_computerdestroy(computer);
}

// }}}
