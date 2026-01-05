#ifndef TIMER_H
#define TIMER_H

#include "types.h"

// PIT Constants
#define PIT_CMD_PORT 0x43
#define PIT_CH0_PORT 0x40
#define PIT_FREQ 1193182

void timer_init();
uint16_t timer_read_count();

#endif // TIMER_H
