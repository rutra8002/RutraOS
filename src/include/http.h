#ifndef HTTP_H
#define HTTP_H

#include "types.h"

// HTTP configuration
#define HTTP_PORT 80
#define HTTPS_PORT 443
#define MAX_URL_LENGTH 512
#define MAX_HEADER_SIZE 4096
#define MAX_RESPONSE_SIZE 65536

// HTTP methods
typedef enum {
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_HEAD
} http_method_t;

// HTTP response structure
typedef struct {
    int status_code;
    char* headers;
    size_t headers_len;
    char* body;
    size_t body_len;
    size_t total_len;
} http_response_t;

// HTTP functions
int http_init(void);
http_response_t* http_get(const char* url);
http_response_t* http_request(const char* url, http_method_t method, const char* body, size_t body_len);
void http_free_response(http_response_t* response);
int http_parse_url(const char* url, char* host, char* path, uint16_t* port);

#endif // HTTP_H
