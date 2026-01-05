#include "timer.h"
#include "io.h"

void timer_init() {
    // Channel 0, Access lo/hi, Mode 2 (Rate Generator), Binary
    outb(PIT_CMD_PORT, 0x34);
    // Set count to 0 (65536)
    outb(PIT_CH0_PORT, 0x00);
    outb(PIT_CH0_PORT, 0x00);
}

uint16_t timer_read_count() {
    uint16_t count = 0;
    outb(PIT_CMD_PORT, 0x00); // Latch
    count = inb(PIT_CH0_PORT);
    count |= (inb(PIT_CH0_PORT) << 8);
    return count;
}
