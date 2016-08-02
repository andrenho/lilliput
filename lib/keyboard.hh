#ifndef KEYBOARD_HH_
#define KEYBOARD_HH_

namespace luisavm {

enum KeyboardModifier { CONTROL=0b1, SHIFT=0b10, ALT=0b100 };
enum Key {
    F1=14, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    INSERT, HOME, DELETE, END, PGUP, PGDOWN,
    LEFT, RIGHT, UP, DOWN,
    ESC=27, TAB=9, BACKSPACE=8,
};

}  // namespace luisavm

#endif
