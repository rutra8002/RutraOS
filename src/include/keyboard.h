#ifndef KEYBOARD_H
#define KEYBOARD_H

// Scancodes
#define KEY_ESC 0x01
#define KEY_W 0x11
#define KEY_A 0x1E
#define KEY_S 0x1F
#define KEY_D 0x20
#define KEY_SPACE 0x39
#define KEY_UP 0x48
#define KEY_LEFT 0x4B
#define KEY_RIGHT 0x4D
#define KEY_DOWN 0x50

#define LEFT_SHIFT_SCANCODE 0x2A
#define RIGHT_SHIFT_SCANCODE 0x36
#define LEFT_SHIFT_RELEASE 0xAA
#define RIGHT_SHIFT_RELEASE 0xB6

void keyboard_init();
char keyboard_getchar();
int keyboard_has_input();
unsigned char keyboard_read_scancode();

#endif
