#include "dns.h"
#include "network.h"
#include "string.h"
#include "memory.h"
#include "memory_utils.h"
#include "terminal.h"

static uint32_t dns_server = DNS_SERVER_IP;
static uint16_t dns_next_id = 1;

// Initialize DNS
int dns_init(void) {
    terminal_writestring("DNS resolver initialized (server: 8.8.8.8)\n");
    return 0;
}

// Set DNS server
void dns_set_server(uint32_t server_ip) {
    dns_server = server_ip;
}

// Encode DNS name (convert "example.com" to length-prefixed format)
static size_t dns_encode_name(const char* hostname, uint8_t* buffer) {
    if (!hostname || !buffer) return 0;
    
    size_t pos = 0;
    const char* label_start = hostname;
    const char* p = hostname;
    
    while (*p) {
        if (*p == '.') {
            size_t label_len = p - label_start;
            if (label_len > 0) {
                buffer[pos++] = (uint8_t)label_len;
                memcpy(buffer + pos, label_start, label_len);
                pos += label_len;
            }
            label_start = p + 1;
        }
        p++;
    }
    
    // Add final label
    size_t label_len = p - label_start;
    if (label_len > 0) {
        buffer[pos++] = (uint8_t)label_len;
        memcpy(buffer + pos, label_start, label_len);
        pos += label_len;
    }
    
    // Add null terminator
    buffer[pos++] = 0;
    
    return pos;
}

// Build DNS query packet
static size_t dns_build_query(const char* hostname, uint8_t* buffer, size_t max_len) {
    if (!hostname || !buffer || max_len < sizeof(dns_header_t) + MAX_HOSTNAME_LENGTH) {
        return 0;
    }
    
    dns_header_t* header = (dns_header_t*)buffer;
    
    // Set header fields
    header->id = dns_next_id++;
    header->flags = 0x0100;  // Standard query, recursion desired
    header->qdcount = 0x0100;  // 1 question (big-endian)
    header->ancount = 0;
    header->nscount = 0;
    header->arcount = 0;
    
    size_t pos = sizeof(dns_header_t);
    
    // Encode hostname
    pos += dns_encode_name(hostname, buffer + pos);
    
    // Add query type (A record) and class (IN)
    uint16_t* qtype = (uint16_t*)(buffer + pos);
    *qtype = 0x0100;  // Type A (big-endian)
    pos += 2;
    
    uint16_t* qclass = (uint16_t*)(buffer + pos);
    *qclass = 0x0100;  // Class IN (big-endian)
    pos += 2;
    
    return pos;
}

// Parse DNS response
static int dns_parse_response(const uint8_t* response, size_t response_len, uint32_t* ip_out) {
    if (!response || response_len < sizeof(dns_header_t) || !ip_out) {
        return -1;
    }
    
    dns_header_t* header = (dns_header_t*)response;
    
    // Check if we got a response
    if ((header->flags & 0x8000) == 0) {
        return -1;  // Not a response
    }
    
    // Check for errors
    uint16_t rcode = header->flags & 0x0F;
    if (rcode != 0) {
        return -1;  // Error in response
    }
    
    // Check if we have answers
    uint16_t ancount = (header->ancount >> 8) | ((header->ancount & 0xFF) << 8);
    if (ancount == 0) {
        return -1;  // No answers
    }
    
    // Skip question section
    size_t pos = sizeof(dns_header_t);
    
    // Skip the encoded name
    while (pos < response_len && response[pos] != 0) {
        uint8_t len = response[pos];
        if ((len & 0xC0) == 0xC0) {
            // Compressed name
            pos += 2;
            break;
        }
        pos += len + 1;
    }
    if (pos < response_len && response[pos] == 0) pos++;
    
    // Skip qtype and qclass
    pos += 4;
    
    // Parse answer section
    while (ancount > 0 && pos < response_len) {
        // Skip name
        if ((response[pos] & 0xC0) == 0xC0) {
            pos += 2;
        } else {
            while (pos < response_len && response[pos] != 0) {
                pos += response[pos] + 1;
            }
            pos++;
        }
        
        if (pos + 10 > response_len) break;
        
        uint16_t type = (response[pos] << 8) | response[pos + 1];
        pos += 2;
        uint16_t class = (response[pos] << 8) | response[pos + 1];
        pos += 2;
        uint32_t ttl = (response[pos] << 24) | (response[pos + 1] << 16) | 
                       (response[pos + 2] << 8) | response[pos + 3];
        pos += 4;
        uint16_t rdlength = (response[pos] << 8) | response[pos + 1];
        pos += 2;
        
        // Check if this is an A record
        if (type == DNS_TYPE_A && rdlength == 4) {
            // Extract IP address (already in network byte order)
            *ip_out = (response[pos] << 24) | (response[pos + 1] << 16) | 
                     (response[pos + 2] << 8) | response[pos + 3];
            return 0;
        }
        
        pos += rdlength;
        ancount--;
    }
    
    return -1;
}

// Resolve hostname to IP address
int dns_resolve(const char* hostname, uint32_t* ip_out) {
    if (!hostname || !ip_out) {
        return -1;
    }
    
    // Check if hostname is already an IP address
    int is_ip = 1;
    for (const char* p = hostname; *p; p++) {
        if (*p != '.' && (*p < '0' || *p > '9')) {
            is_ip = 0;
            break;
        }
    }
    
    if (is_ip) {
        *ip_out = ip_str_to_addr(hostname);
        return 0;
    }
    
    // For simulation purposes, we'll map some common hostnames to IPs
    // In a real implementation, this would send UDP packets to DNS server
    
    if (strcmp(hostname, "example.com") == 0) {
        *ip_out = 0x5DB8D822;  // 93.184.216.34
        return 0;
    } else if (strcmp(hostname, "google.com") == 0) {
        *ip_out = 0x8EFA7BC3;  // 142.250.123.195
        return 0;
    } else if (strcmp(hostname, "github.com") == 0) {
        *ip_out = 0x14FE2E40;  // 20.254.46.64
        return 0;
    } else if (strcmp(hostname, "localhost") == 0) {
        *ip_out = 0x7F000001;  // 127.0.0.1
        return 0;
    }
    
    // Build DNS query
    uint8_t query_buffer[512];
    size_t query_len = dns_build_query(hostname, query_buffer, sizeof(query_buffer));
    
    if (query_len == 0) {
        return -1;
    }
    
    // In real implementation, would send UDP packet to DNS server
    // and wait for response
    
    terminal_writestring("DNS: Resolving ");
    terminal_writestring(hostname);
    terminal_writestring("...\n");
    
    // For simulation, return a fake IP
    *ip_out = 0xC0A80101;  // 192.168.1.1
    
    return -1;  // Indicate resolution failed (simulation)
}
