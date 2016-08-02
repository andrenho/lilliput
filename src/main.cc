#include <cstdint>
#include <cstdlib>
#include <getopt.h>

#include "luisavm.hh"
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <vector>
using namespace std;

// {{{ commandline options

struct Options {
    string   rom_file;
    string   map_file;
    uint32_t memory_size = 16;
    uint8_t  zoom = 2;
    bool     start_with_debugger = true;

    Options(int argc, char* argv[])
    {
        int c;

        while(true) {
            int option_index = 0;
            static struct option long_options[] = {
                {"memory",  required_argument, nullptr,  'M' },
                {"map",     required_argument, nullptr,  'm' },
                {"zoom",    required_argument, nullptr,  'z' },
                {"help",    no_argument,       nullptr,  'h' },
                {nullptr,   0,                 nullptr,   0  }
            };

            c = getopt_long(argc, argv, "m:M:z:h", long_options, &option_index);
            if(c == -1) {
                break;
            }

            switch(c) {
                case 'm':
                    memory_size = strtol(optarg, nullptr, 10);
                    break;
                case 'M':
                    map_file = optarg;
                    break;
                case 'z':
                    zoom = strtol(optarg, nullptr, 10);
                    break;
                case 'h':
                    cout << "LuisaVM emulator version " VERSION "\n";
                    cout << "Options:\n";
                    cout << "   -m, --memory      memory size, in kB\n";
                    cout << "   -z, --zoom        zoom of the display\n";
                    cout << "   -T, --test        run unit tests\n";
                    cout << "   -h, --help        this help\n";
                    exit(EXIT_SUCCESS);
                case '?':
                    exit(EXIT_FAILURE);
                default:
                    abort();
            }
        }

        if(optind < argc) {
            rom_file = argv[optind];
        }
    }
};

// }}}

// {{{ emulator

class Emulator {
public:
    Emulator(int argc, char* argv[]) 
        : opt(argc, argv), comp(opt.memory_size * 1024) 
    {
        zoom = opt.zoom;
        LoadROM();
        InitializeSDL();
        /* luisavm::Video& video = */SetupVideo();
    }


    ~Emulator()
    {
        for(auto& s: sprites) {
            SDL_DestroyTexture(s);
        }
        
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }


    void MainLoop()
    {
        bool active = true;
        while(active) {
            if(GetEvents() == false) {
                active = false;
            }
        }
    }


private:
    void LoadROM() 
    {
        if(opt.rom_file != "") {
            comp.LoadROM(opt.rom_file, opt.map_file);
        }
    }


    void InitializeSDL() 
    {
        if(SDL_Init(SDL_INIT_VIDEO) != 0) {
            cerr << "SDL_Init error: " << SDL_GetError() << "\n";
            exit(EXIT_FAILURE);
        }

        window = SDL_CreateWindow("luisavm " VERSION, 
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                (WIDTH + (2*BORDER)) * zoom, (HEIGHT + (2 * BORDER)) * zoom, 0);
        if(!window) {
            cerr << "SDL_CreateWindow error: " << SDL_GetError() << "\n";
            exit(EXIT_FAILURE);
        }

        ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
        if(!ren) {
            cerr << "SDL_CreateRenderer error: " << SDL_GetError() << "\n";
            exit(EXIT_FAILURE);
        }
        
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 0xFF);
        SDL_RenderClear(ren);
        SDL_RenderPresent(ren);
    }


    luisavm::Video& SetupVideo() 
    {
        // {{{ video functions

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"
        auto setpal = [&](uint8_t idx, uint8_t r, uint8_t g, uint8_t b) {
            pal[idx].r = r;
            pal[idx].g = g;
            pal[idx].b = b;
        };

        auto clrscr = [&](uint8_t color) {
            SDL_SetRenderDrawColor(ren, pal[color].r, pal[color].g, pal[color].b, SDL_ALPHA_OPAQUE);
            SDL_Rect r { BORDER * zoom, BORDER * zoom, WIDTH * zoom, HEIGHT * zoom };
            SDL_RenderFillRect(ren, &r);
        };

        auto change_border_color = [&](uint8_t color) {
            int w, h; SDL_GetWindowSize(window, &w, &h);
            SDL_SetRenderDrawColor(ren, pal[color].r, pal[color].g, pal[color].b, SDL_ALPHA_OPAQUE);
            SDL_Rect r1 = { 0, 0, w, BORDER * zoom },
                     r2 = { 0, h - (BORDER * zoom), w, BORDER * zoom },
                     r3 = { 0, 0, BORDER * zoom, h },
                     r4 = { w - (BORDER * zoom), 0, BORDER * zoom, h };
            SDL_RenderFillRect(ren, &r1);
            SDL_RenderFillRect(ren, &r2);
            SDL_RenderFillRect(ren, &r3);
            SDL_RenderFillRect(ren, &r4);
        };

        auto upload_sprite = [&](uint16_t w, uint16_t h, uint8_t* data) -> uint32_t { 
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

            sprites.push_back(tx);
            return sprites.size();  // n+1
        };

        auto draw_sprite = [&](uint32_t sprite_idx, uint16_t pos_x, uint16_t pos_y) {
            Uint32 format;
            int access, w, h;
            SDL_QueryTexture(sprites.at(sprite_idx-1), &format, &access, &w, &h);
            SDL_Rect r = { (pos_x+BORDER) * zoom, (pos_y+BORDER) * zoom, w * zoom, h * zoom };
            SDL_RenderCopy(ren, sprites.at(sprite_idx-1), nullptr, &r);
        };

        auto update_screen = [&]() {
            SDL_RenderPresent(ren);
        };
#pragma GCC diagnostic pop

        luisavm::Video::Callbacks cb { 
            setpal, clrscr, change_border_color, upload_sprite, draw_sprite,
            update_screen
        };

        // }}}

        return comp.AddDevice<luisavm::Video>(cb);
    }


    bool GetEvents()
    {
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT:
                    return false;
            }
        }
        return true;
    }


    const int WIDTH  = 318;
    const int HEIGHT = 234;
    const int BORDER =  20;

    Options          opt;
    luisavm::LuisaVM comp;

    double               zoom = 2;
    SDL_Window*          window = NULL;
    SDL_Renderer*        ren = NULL;
    SDL_Color            pal[256] = {};
    vector<SDL_Texture*> sprites;
};

// }}}

int main(int argc, char* argv[])
{
    Emulator(argc, argv).MainLoop();
}
