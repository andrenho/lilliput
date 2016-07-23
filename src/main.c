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
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                    uint8_t mod = 0;
                    uint32_t key = e.key.keysym.sym;
                    switch(e.key.keysym.sym) {
                        case SDLK_F1:  key = F1;  break;
                        case SDLK_F2:  key = F2;  break;
                        case SDLK_F3:  key = F3;  break;
                        case SDLK_F4:  key = F4;  break;
                        case SDLK_F5:  key = F5;  break;
                        case SDLK_F6:  key = F6;  break;
                        case SDLK_F7:  key = F7;  break;
                        case SDLK_F8:  key = F8;  break;
                        case SDLK_F9:  key = F9;  break;
                        case SDLK_F10: key = F10; break;
                        case SDLK_F11: key = F11; break;
                        case SDLK_F12: key = F12; break;
                        case SDLK_INSERT:   key = INSERT; break;
                        case SDLK_HOME:     key = HOME;   break;
                        case SDLK_DELETE:   key = DELETE; break;
                        case SDLK_END:      key = END;    break;
                        case SDLK_PAGEDOWN: key = PGUP;   break;
                        case SDLK_PAGEUP:   key = PGDOWN; break;
                        case SDLK_LEFT:     key = LEFT;   break;
                        case SDLK_RIGHT:    key = RIGHT;  break;
                        case SDLK_UP:       key = UP;     break;
                        case SDLK_DOWN:     key = DOWN;   break;
                        case SDLK_LSHIFT: case SDLK_RSHIFT: key = 0; mod = SHIFT; break;
                        case SDLK_LCTRL: case SDLK_RCTRL:   key = 0; mod = CONTROL; break;
                        case SDLK_LALT: case SDLK_RALT:     key = 0; mod = ALT; break;
                            break;
                    }
                    if(key >= 0x40000000) {
                        break;
                    }
                    if(e.key.keysym.mod & KMOD_CTRL)  mod |= CONTROL;
                    if(e.key.keysym.mod & KMOD_SHIFT) mod |= SHIFT;
                    if(e.key.keysym.mod & KMOD_ALT)   mod |= ALT;
                    if(e.key.state == SDL_PRESSED) {
                        lvm_keypressed(comp, key, mod);
                    } else {
                        lvm_keyreleased(comp, key, mod);
                    }
                }
                break;
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
