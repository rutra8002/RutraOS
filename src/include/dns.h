#ifndef DNS_H
#define DNS_H

#include "types.h"

// DNS configuration
#define DNS_SERVER_IP 0x08080808  // 8.8.8.8 (Google DNS)
#define DNS_PORT 53
#define DNS_QUERY_TIMEOUT 5000  // milliseconds
#define MAX_HOSTNAME_LENGTH 255

// DNS header structure
typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} __attribute__((packed)) dns_header_t;

// DNS query types
#define DNS_TYPE_A 1      // IPv4 address
#define DNS_TYPE_AAAA 28  // IPv6 address
#define DNS_TYPE_CNAME 5  // Canonical name

// DNS classes
#define DNS_CLASS_IN 1    // Internet

// DNS functions
int dns_init(void);
int dns_resolve(const char* hostname, uint32_t* ip_out);
void dns_set_server(uint32_t server_ip);

#endif // DNS_H
