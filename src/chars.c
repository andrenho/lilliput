#include "chars.h"

#include <assert.h>
#include <stdlib.h>
#include <syslog.h>
#include <SDL2/SDL.h>

#include "font.h"

typedef struct {
    uint64_t key;
    void*    texture;
} CharRecord;

static CharRecord*   char_list = NULL;
static size_t        char_list_n = 0;
static SDL_Renderer* ren = NULL;
static SDL_Surface*  main_sf = NULL;


void
chars_init(void* data)
{
    ren = (SDL_Renderer*)data;

    // load image bitmap from memory
    SDL_RWops* rw = SDL_RWFromConstMem(data_font_bmp, (int)data_font_bmp_len);
    main_sf = SDL_LoadBMP_RW(rw, 1);  // automatically free rw
    if(!main_sf) {
        syslog(LOG_ERR, "Could not load font image: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    syslog(LOG_DEBUG, "Font loaded.");
}


void 
chars_destroy()
{
    for(size_t i = 0; i < char_list_n; ++i) {
        SDL_DestroyTexture(char_list[i].texture);
    }
    if(char_list) {
        free(char_list);
    }
    SDL_FreeSurface(main_sf);
    syslog(LOG_DEBUG, "Font image destroyed.");
}


static inline uint64_t
char_index(char c, uint8_t fg_r, uint8_t fg_g, uint8_t fg_b, uint8_t bg_r, uint8_t bg_g, uint8_t bg_b)
{
    return ((uint64_t)c << 48) 
         | ((uint64_t)fg_r << 40) | ((uint64_t)fg_g << 32) | ((uint64_t)fg_b << 24) 
         | ((uint64_t)bg_r << 16) | ((uint64_t)bg_g << 8) | (uint64_t)bg_b;
}


static int
compf(const void* cr1, const void* cr2)
{
    uint64_t k1 = ((const CharRecord*)cr1)->key,
             k2 = ((const CharRecord*)cr2)->key;
    if(k1 < k2) {
        return -1;
    } else if(k1 > k2) {
        return 1;
    } else {
        return 0;
    }
}


static inline void*
char_lookup(uint64_t idx)
{
    CharRecord cr = { idx, NULL }, *found;
    found = bsearch(&cr, char_list, char_list_n, sizeof(CharRecord), compf);
    if(found) {
        return found->texture;
    } else {
        return NULL;
    }
}


static void*
char_build(uint64_t idx, char c, uint8_t fg_r, uint8_t fg_g, uint8_t fg_b, uint8_t bg_r, uint8_t bg_g, uint8_t bg_b)
{
    // create texture
    SDL_Texture* tx = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, CHAR_W, CHAR_H);
    SDL_SetRenderTarget(ren, tx);

    // draw background
    SDL_SetRenderDrawColor(ren, bg_r, bg_g, bg_b, 0xFF);
    SDL_RenderFillRect(ren, NULL);

    // draw foreground
    SDL_Point pts[CHAR_W * CHAR_H];
    int n = 0;

    int sx = (c / 16) * CHAR_W;
    int sy = (c % 16) * CHAR_H;
    for(int x = 0; x < CHAR_W; ++x) {
        for(int y = 0; y < CHAR_H; ++y) {
            Uint8* p = (Uint8*)main_sf->pixels + ((y + sy) * main_sf->pitch) + (x + sx);
            if(*p) {
                pts[n++] = (SDL_Point) { x, y };
            }
        }
    }
    SDL_SetRenderDrawColor(ren, fg_r, fg_g, fg_b, 0xFF);
    SDL_RenderDrawPoints(ren, pts, n);

    SDL_SetRenderTarget(ren, NULL);

    // add to array
    char_list = realloc(char_list, (char_list_n+1) * sizeof(CharRecord));
    char_list[char_list_n] = (CharRecord) { idx, (void*)tx };
    ++char_list_n;
    qsort(char_list, char_list_n, sizeof(CharRecord), compf);

    // log
    syslog(LOG_DEBUG, "Character 0x%02X ('%c') created for fg { 0x%02X, 0x%02X, 0x%02X } and bg { 0x%02X, 0x%02X, 0x%02X }.",
            c, (c >= 32 && c < 127) ? c : '?', fg_r, fg_g, fg_b, bg_r, bg_g, bg_b);

    return tx;
}


void* 
chars_get(char c, uint8_t fg_r, uint8_t fg_g, uint8_t fg_b, uint8_t bg_r, uint8_t bg_g, uint8_t bg_b)
{
    uint64_t char_idx = char_index(c, fg_r, fg_g, fg_b, bg_r, bg_g, bg_b);
    void* ch = char_lookup(char_idx);
    if(!ch) {
        ch = char_build(char_idx, c, fg_r, fg_g, fg_b, bg_r, bg_g, bg_b);
        if(!ch) {
            syslog(LOG_ERR, "Could not build character 0x%02X.", c);
            exit(EXIT_FAILURE);
        }
    }
    return ch;
}
