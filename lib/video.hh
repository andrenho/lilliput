#ifndef VIDEO_HH_
#define VIDEO_HH_

#include <cstdint>
#include <functional>
using namespace std;

#include "device.hh"

namespace luisavm {

class Video : public Device {
public:
    struct Callbacks {
        function<void(uint8_t, uint8_t, uint8_t, uint8_t)> setpal;
        function<void(uint8_t)>                            clrscr;
        function<void(uint8_t)>                            change_border_color;
        function<uint32_t(uint16_t, uint16_t, uint8_t*)>   upload_sprite;
        function<void(uint32_t, uint16_t, uint16_t)>       draw_sprite;
        function<void()>                                   update_screen;
    };

    explicit Video(Callbacks const& cb) : cb(cb) {}

private:
    Callbacks cb;
};

}  // namespace luisavm

#endif
