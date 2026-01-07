# RutraOS Networking and Web Browser

This document describes the networking and web browser features added to RutraOS.

## Features Implemented

### 1. Networking Stack

#### Network Driver (`src/drivers/network.c`, `src/include/network.h`)
- **Network Interface Management**: Simulated network interface with MAC and IP configuration
- **Default Configuration**: 
  - IP Address: 10.0.2.15
  - Subnet Mask: 255.255.255.0
  - Gateway: 10.0.2.2 (QEMU default configuration)
- **Socket API**: BSD-style socket interface
  - `socket_create()` - Create new socket
  - `socket_connect()` - Connect to remote host
  - `socket_send()` - Send data
  - `socket_recv()` - Receive data
  - `socket_close()` - Close socket
- **Protocol Support**:
  - TCP (Transmission Control Protocol)
  - UDP (User Datagram Protocol)
  - ARP (Address Resolution Protocol)
  - IP (Internet Protocol v4)
  - ICMP (Internet Control Message Protocol)
- **ARP Cache**: For IP-to-MAC address resolution

#### DNS Resolver (`src/drivers/dns.c`, `src/include/dns.h`)
- **DNS Query Support**: Resolves hostnames to IP addresses
- **Default DNS Server**: 8.8.8.8 (Google Public DNS)
- **Simulated Resolution**: Maps common hostnames to IPs
  - example.com → 93.184.216.34
  - google.com → 142.250.123.195
  - github.com → 20.254.46.64
  - localhost → 127.0.0.1

#### HTTP Client (`src/drivers/http.c`, `src/include/http.h`)
- **HTTP Methods Support**: GET, POST, HEAD
- **URL Parsing**: Extracts host, path, and port from URLs
- **HTTP/1.1 Protocol**: Standard HTTP request/response handling
- **Request Building**: Constructs proper HTTP headers
- **Response Parsing**: Parses status codes, headers, and body

### 2. HTML Rendering Engine

#### HTML Parser (`src/drivers/html.c`, `src/include/html.h`)
- **DOM Tree Construction**: Builds a Document Object Model from HTML source
- **Element Support**:
  - Structural: `<html>`, `<head>`, `<body>`, `<div>`, `<span>`
  - Headers: `<h1>`, `<h2>`, `<h3>`
  - Text: `<p>`, `<br>`, text nodes
  - Formatting: `<b>` (bold), `<i>` (italic), `<u>` (underline)
  - Links: `<a href="...">`
  - Images: `<img src="...">`
- **Text Rendering**: Word-wrapped text output in terminal
- **Formatting**: Simple text-based rendering with markdown-like formatting
  - Headers rendered with `===`, `==`, `=` markers
  - Bold text with `**text**`
  - Italic text with `*text*`

### 3. Web Browser Command

#### Browser Command (`src/commands/utils/browser.c`)
- **Usage**: `browser <url>`
- **Example**: `browser http://example.com`
- **Features**:
  - Fetches web pages via HTTP
  - Parses HTML content
  - Renders pages in text mode
  - Displays HTTP status codes
  - Shows page content with formatting

## Architecture

```
User Command (browser)
    ↓
HTTP Client (http.c)
    ↓
DNS Resolver (dns.c) + Socket API (network.c)
    ↓
Network Driver (network.c)
    ↓
[Simulated Network Interface]
```

## Implementation Details

### Simulated vs Real Implementation

The current implementation is a **simulation** for demonstration purposes:
- Network packets are simulated rather than sent to actual hardware
- HTTP responses are pre-generated for demonstration
- DNS resolution uses a static mapping for common domains

### For Production Use

To make this production-ready, the following would need to be implemented:
1. **Network Hardware Driver**: Support for actual NICs (e.g., RTL8139, E1000)
2. **Packet Transmission**: Real packet send/receive via DMA
3. **Interrupt Handling**: Network card interrupts for packet arrival
4. **TCP State Machine**: Full TCP connection state management
5. **DNS Protocol**: Actual UDP-based DNS queries
6. **SSL/TLS**: HTTPS support for secure connections

## Testing

### Using the Browser Command

1. Build and run RutraOS:
   ```bash
   make clean
   make all
   make run
   ```

2. At the RutraOS prompt, type:
   ```
   browser http://example.com
   ```

3. The browser will:
   - Initialize the network stack
   - Resolve the hostname
   - Connect to the server
   - Fetch the webpage
   - Parse the HTML
   - Render the content

### Expected Output

```
====================================
   RutraOS Web Browser v1.0
====================================

Fetching: http://example.com

HTTP Status: 200 OK

------------------------------------
         Rendering Page
------------------------------------

=== Welcome to RutraOS Web Browser! ===

This is a simulated webpage rendered in RutraOS.

The networking stack is operational!

== Features: ==

- HTML rendering
- TCP/IP stack
- DNS resolution

------------------------------------
      End of Page
------------------------------------
```

## Future Enhancements

1. **Graphics Mode Rendering**: Render HTML in VGA graphics mode with proper layout
2. **CSS Support**: Basic CSS styling support
3. **JavaScript**: Simple JavaScript interpreter
4. **Image Support**: Display images (GIF, JPEG, PNG)
5. **Form Support**: HTML forms and POST requests
6. **Cookies**: Cookie management
7. **Cache**: Page caching for faster loading
8. **Bookmarks**: Save favorite pages
9. **History**: Browse history tracking
10. **Tabs**: Multiple page support

## Technical Notes

- All networking code is located in `src/drivers/`
- Headers are in `src/include/`
- The browser command is in `src/commands/utils/`
- Network initialization happens in `kernel_main()` during boot
- The browser command is registered in `command_register_builtins()`

## Credits

Implemented as part of RutraOS development to provide advanced networking capabilities and web browsing functionality.
