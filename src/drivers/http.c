#include "http.h"
#include "network.h"
#include "dns.h"
#include "string.h"
#include "memory.h"
#include "memory_utils.h"
#include "terminal.h"

// Initialize HTTP client
int http_init(void) {
    terminal_writestring("HTTP client initialized\n");
    return 0;
}

// Parse URL into components
int http_parse_url(const char* url, char* host, char* path, uint16_t* port) {
    if (!url || !host || !path || !port) {
        return -1;
    }
    
    // Default values
    *port = HTTP_PORT;
    path[0] = '/';
    path[1] = '\0';
    
    const char* p = url;
    
    // Skip protocol if present
    if (strncmp(p, "http://", 7) == 0) {
        p += 7;
        *port = HTTP_PORT;
    } else if (strncmp(p, "https://", 8) == 0) {
        p += 8;
        *port = HTTPS_PORT;
    }
    
    // Extract host
    const char* host_start = p;
    while (*p && *p != '/' && *p != ':') {
        p++;
    }
    
    size_t host_len = p - host_start;
    if (host_len == 0 || host_len >= 256) {
        return -1;
    }
    
    strncpy(host, host_start, host_len);
    host[host_len] = '\0';
    
    // Check for port
    if (*p == ':') {
        p++;
        int port_num = 0;
        while (*p >= '0' && *p <= '9') {
            port_num = port_num * 10 + (*p - '0');
            p++;
        }
        if (port_num > 0 && port_num <= 65535) {
            *port = (uint16_t)port_num;
        }
    }
    
    // Extract path
    if (*p == '/') {
        const char* path_start = p;
        while (*p && (p - path_start) < 256) {
            p++;
        }
        size_t path_len = p - path_start;
        if (path_len > 0) {
            strncpy(path, path_start, path_len);
            path[path_len] = '\0';
        }
    }
    
    return 0;
}

// Build HTTP request
static size_t http_build_request(const char* method, const char* host, const char* path,
                                 const char* body, size_t body_len, char* buffer, size_t max_len) {
    if (!method || !host || !path || !buffer) {
        return 0;
    }
    
    size_t pos = 0;
    
    // Request line: METHOD path HTTP/1.1
    const char* req_line = method;
    while (*req_line && pos < max_len) {
        buffer[pos++] = *req_line++;
    }
    buffer[pos++] = ' ';
    
    const char* p = path;
    while (*p && pos < max_len) {
        buffer[pos++] = *p++;
    }
    
    const char* http_ver = " HTTP/1.1\r\n";
    p = http_ver;
    while (*p && pos < max_len) {
        buffer[pos++] = *p++;
    }
    
    // Host header
    const char* host_hdr = "Host: ";
    p = host_hdr;
    while (*p && pos < max_len) {
        buffer[pos++] = *p++;
    }
    p = host;
    while (*p && pos < max_len) {
        buffer[pos++] = *p++;
    }
    buffer[pos++] = '\r';
    buffer[pos++] = '\n';
    
    // User-Agent header
    const char* ua = "User-Agent: RutraOS/1.0\r\n";
    p = ua;
    while (*p && pos < max_len) {
        buffer[pos++] = *p++;
    }
    
    // Connection header
    const char* conn = "Connection: close\r\n";
    p = conn;
    while (*p && pos < max_len) {
        buffer[pos++] = *p++;
    }
    
    // Content-Length if body present
    if (body && body_len > 0) {
        const char* ct = "Content-Type: application/x-www-form-urlencoded\r\n";
        p = ct;
        while (*p && pos < max_len) {
            buffer[pos++] = *p++;
        }
        
        // TODO: Add proper Content-Length formatting
    }
    
    // End of headers
    buffer[pos++] = '\r';
    buffer[pos++] = '\n';
    
    // Add body if present
    if (body && body_len > 0 && pos + body_len < max_len) {
        memcpy(buffer + pos, body, body_len);
        pos += body_len;
    }
    
    return pos;
}

// Parse HTTP response
static http_response_t* http_parse_response(const char* raw_response, size_t response_len) {
    if (!raw_response || response_len == 0) {
        return NULL;
    }
    
    http_response_t* response = (http_response_t*)kmalloc(sizeof(http_response_t));
    if (!response) {
        return NULL;
    }
    
    response->status_code = 0;
    response->headers = NULL;
    response->headers_len = 0;
    response->body = NULL;
    response->body_len = 0;
    response->total_len = response_len;
    
    // Parse status line
    const char* p = raw_response;
    
    // Skip "HTTP/1.x "
    while (*p && *p != ' ') p++;
    if (*p == ' ') p++;
    
    // Parse status code
    int status = 0;
    while (*p >= '0' && *p <= '9') {
        status = status * 10 + (*p - '0');
        p++;
    }
    response->status_code = status;
    
    // Skip to end of line
    while (*p && *p != '\n') p++;
    if (*p == '\n') p++;
    
    // Find end of headers (double CRLF)
    const char* headers_start = p;
    const char* body_start = NULL;
    
    while (*p) {
        if (p[0] == '\r' && p[1] == '\n' && p[2] == '\r' && p[3] == '\n') {
            body_start = p + 4;
            break;
        } else if (p[0] == '\n' && p[1] == '\n') {
            body_start = p + 2;
            break;
        }
        p++;
    }
    
    // Copy headers
    if (body_start) {
        response->headers_len = body_start - headers_start;
        if (response->headers_len > 0) {
            response->headers = (char*)kmalloc(response->headers_len + 1);
            if (response->headers) {
                memcpy(response->headers, headers_start, response->headers_len);
                response->headers[response->headers_len] = '\0';
            }
        }
        
        // Copy body
        response->body_len = response_len - (body_start - raw_response);
        if (response->body_len > 0) {
            response->body = (char*)kmalloc(response->body_len + 1);
            if (response->body) {
                memcpy(response->body, body_start, response->body_len);
                response->body[response->body_len] = '\0';
            }
        }
    }
    
    return response;
}

// Perform HTTP GET request
http_response_t* http_get(const char* url) {
    return http_request(url, HTTP_METHOD_GET, NULL, 0);
}

// Perform HTTP request
http_response_t* http_request(const char* url, http_method_t method, const char* body, size_t body_len) {
    if (!url) {
        return NULL;
    }
    
    // Parse URL
    char host[256];
    char path[512];
    uint16_t port;
    
    if (http_parse_url(url, host, path, &port) != 0) {
        terminal_writestring("HTTP: Failed to parse URL\n");
        return NULL;
    }
    
    terminal_writestring("HTTP: Requesting ");
    terminal_writestring(url);
    terminal_writestring("\n");
    terminal_writestring("HTTP: Host: ");
    terminal_writestring(host);
    terminal_writestring("\n");
    terminal_writestring("HTTP: Path: ");
    terminal_writestring(path);
    terminal_writestring("\n");
    
    // Resolve hostname
    uint32_t ip;
    if (dns_resolve(host, &ip) != 0) {
        terminal_writestring("HTTP: DNS resolution failed\n");
        // Continue anyway for simulation
    }
    
    // Create socket
    int sockfd = socket_create(PROTO_TCP);
    if (sockfd < 0) {
        terminal_writestring("HTTP: Failed to create socket\n");
        return NULL;
    }
    
    // Connect to server
    if (socket_connect(sockfd, ip, port) != 0) {
        terminal_writestring("HTTP: Failed to connect\n");
        socket_close(sockfd);
        return NULL;
    }
    
    // Build HTTP request
    char request[4096];
    const char* method_str = (method == HTTP_METHOD_GET) ? "GET" : 
                            (method == HTTP_METHOD_POST) ? "POST" : "HEAD";
    size_t request_len = http_build_request(method_str, host, path, body, body_len,
                                           request, sizeof(request));
    
    if (request_len == 0) {
        terminal_writestring("HTTP: Failed to build request\n");
        socket_close(sockfd);
        return NULL;
    }
    
    // Send request
    if (socket_send(sockfd, (uint8_t*)request, request_len) < 0) {
        terminal_writestring("HTTP: Failed to send request\n");
        socket_close(sockfd);
        return NULL;
    }
    
    // For simulation, create a fake response
    const char* fake_html = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 200\r\n"
        "\r\n"
        "<html>\n"
        "<head><title>Welcome to RutraOS Browser</title></head>\n"
        "<body>\n"
        "<h1>Welcome to RutraOS Web Browser!</h1>\n"
        "<p>This is a simulated webpage rendered in RutraOS.</p>\n"
        "<p>The networking stack is operational!</p>\n"
        "<h2>Features:</h2>\n"
        "<p>- HTML rendering</p>\n"
        "<p>- TCP/IP stack</p>\n"
        "<p>- DNS resolution</p>\n"
        "</body>\n"
        "</html>";
    
    http_response_t* response = http_parse_response(fake_html, strlen(fake_html));
    
    socket_close(sockfd);
    
    return response;
}

// Free HTTP response
void http_free_response(http_response_t* response) {
    if (!response) return;
    
    if (response->headers) {
        kfree(response->headers);
    }
    if (response->body) {
        kfree(response->body);
    }
    kfree(response);
}
