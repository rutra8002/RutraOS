#ifndef NETWORK_H
#define NETWORK_H

#include "types.h"

// Network configuration
#define MAX_PACKET_SIZE 1500
#define MAX_SOCKETS 16
#define ARP_CACHE_SIZE 32

// Ethernet frame structure
typedef struct {
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
    uint8_t payload[MAX_PACKET_SIZE];
} __attribute__((packed)) ethernet_frame_t;

// IP header structure
typedef struct {
    uint8_t version_ihl;
    uint8_t tos;
    uint16_t total_length;
    uint16_t id;
    uint16_t flags_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dest_ip;
} __attribute__((packed)) ip_header_t;

// TCP header structure
typedef struct {
    uint16_t src_port;
    uint16_t dest_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint16_t flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent_ptr;
} __attribute__((packed)) tcp_header_t;

// UDP header structure
typedef struct {
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed)) udp_header_t;

// ARP packet structure
typedef struct {
    uint16_t hw_type;
    uint16_t proto_type;
    uint8_t hw_addr_len;
    uint8_t proto_addr_len;
    uint16_t operation;
    uint8_t sender_hw_addr[6];
    uint32_t sender_proto_addr;
    uint8_t target_hw_addr[6];
    uint32_t target_proto_addr;
} __attribute__((packed)) arp_packet_t;

// Socket states
typedef enum {
    SOCKET_STATE_CLOSED,
    SOCKET_STATE_LISTEN,
    SOCKET_STATE_SYN_SENT,
    SOCKET_STATE_SYN_RECEIVED,
    SOCKET_STATE_ESTABLISHED,
    SOCKET_STATE_FIN_WAIT_1,
    SOCKET_STATE_FIN_WAIT_2,
    SOCKET_STATE_CLOSE_WAIT,
    SOCKET_STATE_CLOSING,
    SOCKET_STATE_LAST_ACK,
    SOCKET_STATE_TIME_WAIT
} socket_state_t;

// Socket structure
typedef struct {
    int used;
    int protocol;  // 6 = TCP, 17 = UDP
    uint32_t local_ip;
    uint16_t local_port;
    uint32_t remote_ip;
    uint16_t remote_port;
    socket_state_t state;
    uint8_t rx_buffer[4096];
    size_t rx_len;
    uint32_t seq_num;
    uint32_t ack_num;
} socket_t;

// Network interface structure
typedef struct {
    uint8_t mac_addr[6];
    uint32_t ip_addr;
    uint32_t subnet_mask;
    uint32_t gateway;
    int link_up;
} network_interface_t;

// Protocol numbers
#define PROTO_ICMP 1
#define PROTO_TCP 6
#define PROTO_UDP 17

// Ethertype values
#define ETHERTYPE_IP 0x0800
#define ETHERTYPE_ARP 0x0806

// ARP operations
#define ARP_REQUEST 1
#define ARP_REPLY 2

// TCP flags
#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20

// Network functions
void network_init(void);
int network_send_packet(const uint8_t* data, size_t len, uint32_t dest_ip, uint16_t dest_port, int protocol);
int network_receive_packet(uint8_t* buffer, size_t max_len, uint32_t* src_ip, uint16_t* src_port);
void network_process_packets(void);

// Socket API
int socket_create(int protocol);
int socket_connect(int sockfd, uint32_t ip, uint16_t port);
int socket_send(int sockfd, const uint8_t* data, size_t len);
int socket_recv(int sockfd, uint8_t* buffer, size_t max_len);
void socket_close(int sockfd);

// ARP functions
void arp_send_request(uint32_t target_ip);
int arp_resolve(uint32_t ip, uint8_t* mac_out);

// IP utilities
uint32_t ip_str_to_addr(const char* ip_str);
void ip_addr_to_str(uint32_t ip, char* buffer);
uint16_t network_checksum(const void* data, size_t len);

// Network interface management
network_interface_t* network_get_interface(void);
void network_set_ip(uint32_t ip, uint32_t subnet, uint32_t gateway);

#endif // NETWORK_H
