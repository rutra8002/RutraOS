#include "command.h"
#include "rtc.h"
#include "terminal.h"
#include "string.h"

int cmd_date_main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    rtc_time_t time = rtc_get_time();
    char buffer[32];

    terminal_writestring("Current Date & Time: 20");
    
    // Year
    uint32_to_string_padded(time.year, buffer, 2, '0');
    terminal_writestring(buffer);
    terminal_writestring("-");

    // Month
    uint32_to_string_padded(time.month, buffer, 2, '0');
    terminal_writestring(buffer);
    terminal_writestring("-");

    // Day
    uint32_to_string_padded(time.day, buffer, 2, '0');
    terminal_writestring(buffer);
    terminal_writestring(" ");

    // Hour
    uint32_to_string_padded(time.hour, buffer, 2, '0');
    terminal_writestring(buffer);
    terminal_writestring(":");

    // Minute
    uint32_to_string_padded(time.minute, buffer, 2, '0');
    terminal_writestring(buffer);
    terminal_writestring(":");

    // Second
    uint32_to_string_padded(time.second, buffer, 2, '0');
    terminal_writestring(buffer);
    terminal_writestring("\n");

    return 0;
}

const command_info_t cmd_info_cmd_date_main = {
    .name = "date",
    .description = "Display current date and time from RTC",
    .main = cmd_date_main,
    .version = 1
};
