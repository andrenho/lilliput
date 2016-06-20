#ifndef CHARS_H_
#define CHARS_H_

#include <stdint.h>

#define CHAR_W 6
#define CHAR_H 9

void chars_init(void* data);
void chars_destroy();

void* chars_get(char c, uint8_t fg_r, uint8_t fg_g, uint8_t fg_b, 
        uint8_t bg_r, uint8_t bg_g, uint8_t bg_b);

#endif
