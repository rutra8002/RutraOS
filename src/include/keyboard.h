#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_init();
char keyboard_getchar();
int keyboard_has_input();
unsigned char keyboard_read_scancode();

#endif
