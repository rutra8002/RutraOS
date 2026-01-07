#include "command.h"
#include "terminal.h"
#include "network.h"
#include "dns.h"
#include "http.h"
#include "html.h"
#include "keyboard.h"
#include "string.h"

// Browser command implementation
int cmd_browser_main(int argc, char** argv) {
    if (command_check_help_flag(argc, argv)) {
        command_show_usage("browser", "<url>");
        terminal_writestring("Browse a website and display its HTML content.\n");
        terminal_writestring("Example: browser http://example.com\n");
        return 0;
    }
    
    if (argc < 2) {
        terminal_writestring("Usage: browser <url>\n");
        terminal_writestring("Example: browser http://example.com\n");
        return 1;
    }
    
    const char* url = argv[1];
    
    terminal_writestring("\n");
    terminal_writestring("====================================\n");
    terminal_writestring("   RutraOS Web Browser v1.0\n");
    terminal_writestring("====================================\n");
    terminal_writestring("\n");
    
    terminal_writestring("Fetching: ");
    terminal_writestring(url);
    terminal_writestring("\n\n");
    
    // Fetch the webpage
    http_response_t* response = http_get(url);
    
    if (!response) {
        terminal_writestring("Error: Failed to fetch webpage\n");
        return 1;
    }
    
    terminal_writestring("HTTP Status: ");
    if (response->status_code == 200) {
        terminal_writestring("200 OK\n");
    } else {
        terminal_writestring("Error\n");
    }
    
    terminal_writestring("\n");
    terminal_writestring("------------------------------------\n");
    terminal_writestring("         Rendering Page\n");
    terminal_writestring("------------------------------------\n");
    terminal_writestring("\n");
    
    // Parse HTML
    if (response->body && response->body_len > 0) {
        html_document_t* doc = html_parse(response->body);
        
        if (doc) {
            // Render the HTML
            html_render_state_t render_state;
            html_render(doc, &render_state);
            
            // Free document
            html_free_document(doc);
        } else {
            terminal_writestring("Error: Failed to parse HTML\n");
        }
    } else {
        terminal_writestring("Error: Empty response body\n");
    }
    
    terminal_writestring("\n");
    terminal_writestring("------------------------------------\n");
    terminal_writestring("      End of Page\n");
    terminal_writestring("------------------------------------\n");
    
    // Free response
    http_free_response(response);
    
    return 0;
}

REGISTER_COMMAND("browser", "Browse and display HTML websites", cmd_browser_main);
