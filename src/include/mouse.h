#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"

void mouse_init();
void mouse_handle_byte(uint8_t data);
int mouse_get_x();
int mouse_get_y();
int mouse_get_buttons();

#endif
