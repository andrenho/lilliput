#ifndef KEYBOARD_HH_
#define KEYBOARD_HH_

#include <cstdint>
#include <deque>
using namespace std;

#include "device.hh"

namespace luisavm {

enum KeyboardModifier : uint8_t { NONE=0, CONTROL=0b1, SHIFT=0b10, ALT=0b100 };
enum Key : uint32_t {
    F1=14, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    INSERT, HOME=28, DELETE, END, PGUP, PGDOWN,
    LEFT, RIGHT, UP, DOWN,
    ESC=27, TAB=9, BACKSPACE=8, ENTER=13,
};
enum KeyState { PRESSED, RELEASED };


class Keyboard : public Device {
public:
    struct KeyPress {
        uint32_t         key;
        KeyboardModifier mod;
        KeyState         state;
    };

    deque<KeyPress> Queue;

    void Reset() override { Queue = decltype(Queue)(); }
};

}  // namespace luisavm

#endif
