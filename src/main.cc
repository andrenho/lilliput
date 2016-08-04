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
            comp.Step();
            SDL_Delay(1);
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

        return comp.AddVideo(cb);
    }


    bool GetEvents()
    {
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT:
                    return false;
                // {{{ keyboard events
                case SDL_KEYDOWN:
                case SDL_KEYUP: {
                        uint8_t mod = 0;
                        uint32_t key = e.key.keysym.sym;
                        switch(e.key.keysym.sym) {
                            case SDLK_F1:       key = luisavm::F1;  break;
                            case SDLK_F2:       key = luisavm::F2;  break;
                            case SDLK_F3:       key = luisavm::F3;  break;
                            case SDLK_F4:       key = luisavm::F4;  break;
                            case SDLK_F5:       key = luisavm::F5;  break;
                            case SDLK_F6:       key = luisavm::F6;  break;
                            case SDLK_F7:       key = luisavm::F7;  break;
                            case SDLK_F8:       key = luisavm::F8;  break;
                            case SDLK_F9:       key = luisavm::F9;  break;
                            case SDLK_F10:      key = luisavm::F10; break;
                            case SDLK_F11:      key = luisavm::F11; break;
                            case SDLK_F12:      key = luisavm::F12; break;
                            case SDLK_INSERT:   key = luisavm::INSERT; break;
                            case SDLK_HOME:     key = luisavm::HOME;   break;
                            case SDLK_DELETE:   key = luisavm::DELETE; break;
                            case SDLK_END:      key = luisavm::END;    break;
                            case SDLK_PAGEDOWN: key = luisavm::PGDOWN; break;
                            case SDLK_PAGEUP:   key = luisavm::PGUP;   break;
                            case SDLK_LEFT:     key = luisavm::LEFT;   break;
                            case SDLK_RIGHT:    key = luisavm::RIGHT;  break;
                            case SDLK_UP:       key = luisavm::UP;     break;
                            case SDLK_DOWN:     key = luisavm::DOWN;   break;
                            case SDLK_RETURN:   key = luisavm::ENTER;  break;
                            case SDLK_LSHIFT: case SDLK_RSHIFT: key = 0; mod = luisavm::SHIFT; break;
                            case SDLK_LCTRL:  case SDLK_RCTRL:  key = 0; mod = luisavm::CONTROL; break;
                            case SDLK_LALT:   case SDLK_RALT:   key = 0; mod = luisavm::ALT; break;
                                break;
                        }
                        if(key >= 0x40000000) {
                            break;
                        }
                        if(e.key.keysym.mod & KMOD_CTRL)  mod |= luisavm::CONTROL;
                        if(e.key.keysym.mod & KMOD_SHIFT) mod |= luisavm::SHIFT;
                        if(e.key.keysym.mod & KMOD_ALT)   mod |= luisavm::ALT;
                        luisavm::KeyState ks = (e.key.state == SDL_PRESSED) ? luisavm::PRESSED : luisavm::RELEASED;
                        comp.keyboard().Queue.push({ 
                            key, 
                            static_cast<luisavm::KeyboardModifier>(mod), 
                            ks 
                        });
                    }
                    break;
                // }}}
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
