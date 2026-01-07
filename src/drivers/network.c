#include "network.h"
#include "terminal.h"
#include "string.h"
#include "memory.h"
#include "memory_utils.h"
#include "io.h"

// Global network interface
static network_interface_t net_interface;
static socket_t sockets[MAX_SOCKETS];

// ARP cache
typedef struct {
    uint32_t ip;
    uint8_t mac[6];
    int valid;
} arp_cache_entry_t;

static arp_cache_entry_t arp_cache[ARP_CACHE_SIZE];

// Initialize network subsystem
void network_init(void) {
    // Set default MAC address (simulated)
    net_interface.mac_addr[0] = 0x52;
    net_interface.mac_addr[1] = 0x54;
    net_interface.mac_addr[2] = 0x00;
    net_interface.mac_addr[3] = 0x12;
    net_interface.mac_addr[4] = 0x34;
    net_interface.mac_addr[5] = 0x56;
    
    // Set default IP configuration (10.0.2.15 - QEMU default)
    net_interface.ip_addr = 0x0A00020F;  // 10.0.2.15
    net_interface.subnet_mask = 0xFFFFFF00;  // 255.255.255.0
    net_interface.gateway = 0x0A000202;  // 10.0.2.2
    net_interface.link_up = 1;
    
    // Initialize sockets
    for (int i = 0; i < MAX_SOCKETS; i++) {
        sockets[i].used = 0;
        sockets[i].state = SOCKET_STATE_CLOSED;
    }
    
    // Initialize ARP cache
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        arp_cache[i].valid = 0;
    }
    
    terminal_writestring("Network initialized\n");
    terminal_writestring("IP: 10.0.2.15\n");
    terminal_writestring("Gateway: 10.0.2.2\n");
}

// Get network interface
network_interface_t* network_get_interface(void) {
    return &net_interface;
}

// Set IP configuration
void network_set_ip(uint32_t ip, uint32_t subnet, uint32_t gateway) {
    net_interface.ip_addr = ip;
    net_interface.subnet_mask = subnet;
    net_interface.gateway = gateway;
}

// Convert IP string to address (e.g., "192.168.1.1" -> 0xC0A80101)
uint32_t ip_str_to_addr(const char* ip_str) {
    if (!ip_str) return 0;
    
    uint32_t result = 0;
    int octet = 0;
    int shift = 24;
    
    for (const char* p = ip_str; *p; p++) {
        if (*p >= '0' && *p <= '9') {
            octet = octet * 10 + (*p - '0');
        } else if (*p == '.') {
            result |= (octet << shift);
            shift -= 8;
            octet = 0;
        }
    }
    result |= (octet << shift);
    
    return result;
}

// Convert IP address to string
void ip_addr_to_str(uint32_t ip, char* buffer) {
    if (!buffer) return;
    
    uint8_t* bytes = (uint8_t*)&ip;
    
    // Format: a.b.c.d
    int pos = 0;
    for (int i = 3; i >= 0; i--) {
        uint8_t byte = bytes[i];
        if (byte >= 100) {
            buffer[pos++] = '0' + (byte / 100);
            byte %= 100;
        }
        if (byte >= 10 || bytes[i] >= 100) {
            buffer[pos++] = '0' + (byte / 10);
            byte %= 10;
        }
        buffer[pos++] = '0' + byte;
        if (i > 0) buffer[pos++] = '.';
    }
    buffer[pos] = '\0';
}

// Calculate network checksum
uint16_t network_checksum(const void* data, size_t len) {
    const uint16_t* ptr = (const uint16_t*)data;
    uint32_t sum = 0;
    
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    
    if (len > 0) {
        sum += *(uint8_t*)ptr;
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

// Create a socket
int socket_create(int protocol) {
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (!sockets[i].used) {
            sockets[i].used = 1;
            sockets[i].protocol = protocol;
            sockets[i].state = SOCKET_STATE_CLOSED;
            sockets[i].local_ip = net_interface.ip_addr;
            sockets[i].local_port = 30000 + i;
            sockets[i].rx_len = 0;
            sockets[i].seq_num = 1000;
            sockets[i].ack_num = 0;
            return i;
        }
    }
    return -1;
}

// Connect socket
int socket_connect(int sockfd, uint32_t ip, uint16_t port) {
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || !sockets[sockfd].used) {
        return -1;
    }
    
    socket_t* sock = &sockets[sockfd];
    sock->remote_ip = ip;
    sock->remote_port = port;
    
    if (sock->protocol == PROTO_TCP) {
        // Simulate TCP handshake
        sock->state = SOCKET_STATE_SYN_SENT;
        // In real implementation, would send SYN packet
        sock->state = SOCKET_STATE_ESTABLISHED;
    } else {
        sock->state = SOCKET_STATE_ESTABLISHED;
    }
    
    return 0;
}

// Send data through socket
int socket_send(int sockfd, const uint8_t* data, size_t len) {
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || !sockets[sockfd].used) {
        return -1;
    }
    
    socket_t* sock = &sockets[sockfd];
    if (sock->state != SOCKET_STATE_ESTABLISHED) {
        return -1;
    }
    
    return network_send_packet(data, len, sock->remote_ip, sock->remote_port, sock->protocol);
}

// Receive data from socket
int socket_recv(int sockfd, uint8_t* buffer, size_t max_len) {
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || !sockets[sockfd].used) {
        return -1;
    }
    
    socket_t* sock = &sockets[sockfd];
    
    if (sock->rx_len == 0) {
        return 0;  // No data available
    }
    
    size_t to_copy = (sock->rx_len < max_len) ? sock->rx_len : max_len;
    memcpy(buffer, sock->rx_buffer, to_copy);
    
    // Remove copied data from buffer
    if (to_copy < sock->rx_len) {
        memmove(sock->rx_buffer, sock->rx_buffer + to_copy, sock->rx_len - to_copy);
    }
    sock->rx_len -= to_copy;
    
    return to_copy;
}

// Close socket
void socket_close(int sockfd) {
    if (sockfd < 0 || sockfd >= MAX_SOCKETS) {
        return;
    }
    
    socket_t* sock = &sockets[sockfd];
    if (sock->protocol == PROTO_TCP && sock->state == SOCKET_STATE_ESTABLISHED) {
        sock->state = SOCKET_STATE_FIN_WAIT_1;
        // In real implementation, would send FIN packet
    }
    
    sock->used = 0;
    sock->state = SOCKET_STATE_CLOSED;
    sock->rx_len = 0;
}

// Send ARP request
void arp_send_request(uint32_t target_ip) {
    // In a real implementation, this would send an ARP request packet
    // For simulation, we'll just add a fake entry
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (!arp_cache[i].valid) {
            arp_cache[i].ip = target_ip;
            // Fake MAC address
            arp_cache[i].mac[0] = 0x52;
            arp_cache[i].mac[1] = 0x54;
            arp_cache[i].mac[2] = 0x00;
            arp_cache[i].mac[3] = (target_ip >> 16) & 0xFF;
            arp_cache[i].mac[4] = (target_ip >> 8) & 0xFF;
            arp_cache[i].mac[5] = target_ip & 0xFF;
            arp_cache[i].valid = 1;
            break;
        }
    }
}

// Resolve IP to MAC address
int arp_resolve(uint32_t ip, uint8_t* mac_out) {
    if (!mac_out) return -1;
    
    // Check cache first
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].valid && arp_cache[i].ip == ip) {
            memcpy(mac_out, arp_cache[i].mac, 6);
            return 0;
        }
    }
    
    // Send ARP request
    arp_send_request(ip);
    
    // In simulation, assume it succeeded
    return arp_resolve(ip, mac_out);
}

// Send network packet (simulated)
int network_send_packet(const uint8_t* data, size_t len, uint32_t dest_ip, uint16_t dest_port, int protocol) {
    if (!data || len == 0) return -1;
    
    // In a real implementation, this would:
    // 1. Build Ethernet frame
    // 2. Build IP header
    // 3. Build TCP/UDP header
    // 4. Send to network device
    
    // For simulation, we just return success
    return len;
}

// Process incoming packets (simulated)
void network_process_packets(void) {
    // In a real implementation, this would:
    // 1. Check network device for incoming packets
    // 2. Parse Ethernet frame
    // 3. Handle ARP, IP, TCP, UDP
    // 4. Deliver data to appropriate socket
    
    // For simulation, this is a no-op
}

// Receive packet (simulated)
int network_receive_packet(uint8_t* buffer, size_t max_len, uint32_t* src_ip, uint16_t* src_port) {
    // In a real implementation, this would receive from network device
    // For simulation, we return 0 (no data)
    return 0;
}
