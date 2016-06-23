#ifndef COMPUTER_H_
#define COMPUTER_H_

#include <stdbool.h>
#include "config.h"

void computer_init(Config* config);
void computer_destroy();

bool computer_active();
void computer_step();
void computer_videoupdate();
void computer_reset();

#endif
