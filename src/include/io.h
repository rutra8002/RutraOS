#ifndef IO_H
#define IO_H

unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
unsigned short inw(unsigned short port);

#endif
