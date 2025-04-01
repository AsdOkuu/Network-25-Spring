#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <arpa/inet.h>
#include <pcap.h>
#include <pthread.h>

#define MAX_DEVICES 16
#define MAX_PACKETS 1024
#define PACKET_BUF_SIZE 2048

#define MTU 1000

// Ethernet frame header
typedef struct {
    uint8_t dst_mac[6];    // Destination MAC address
    uint8_t src_mac[6];    // Source MAC address
    uint16_t ether_type;   // Ethernet type (e.g. 0x0800 for IP)
} __attribute__((packed)) eth_header_t;

// IP protocol header
typedef struct {
    uint8_t  version_ihl;  // Version (4 bits) + IHL (4 bits)
    uint8_t  tos;          // Type of service
    uint16_t total_len;    // Total length
    uint16_t id;           // Identification
    uint16_t frag_off;     // Flags (3 bits) + Fragment offset (13 bits)
    uint8_t  ttl;          // Time to live
    uint8_t  protocol;     // Protocol (TCP=6, UDP=17)
    uint16_t checksum;     // Header checksum
    uint32_t src_ip;       // Source IP address
    uint32_t dst_ip;       // Destination IP address
} __attribute__((packed)) ip_header_t;




/* Network device information */
typedef struct {
    char name[32];          // Device name
    pcap_t *handle;         // pcap handle
    pthread_t thread_id;    // Capture thread ID
    int index;              // Device index
} net_device_t;

/* Packet buffer entry */
typedef struct {
    net_device_t *device;   // Ingress device
    uint8_t data[PACKET_BUF_SIZE]; // Packet data
    uint32_t len;           // Packet length
    uint64_t timestamp;     // Capture timestamp
} packet_entry_t;

/* Global packet buffer */
typedef struct {
    packet_entry_t packets[MAX_PACKETS];
    int head;
    int tail;
    pthread_mutex_t lock;
} packet_buffer_t;

/* Function declarations */
int common_init(void);
void *capture_thread(void *arg);
int send_packet(net_device_t *dev, const uint8_t *data, uint32_t len);
uint64_t get_current_time_us(void);


/************************************
 *  Switch
 ***********************************/
 
/* MAC address to device mapping entry */
typedef struct {
    uint8_t mac[6];         // MAC address
    char device[32];        // Device name
} fdb_entry_t;

/* Forwarding database (MAC learning table) */
typedef struct {
    fdb_entry_t entries[MAX_DEVICES];
    int count;
} forwarding_db_t;

 
/************************************
 *  Router
 ***********************************/

/* Routing table entry */
typedef struct {
    uint32_t dest_ip;        // Destination IP address
    uint32_t mask;           // Network mask
    char out_dev[32];        // Output device name
    uint32_t distance;       // Distance metric
} route_entry_t;

/* Distance Vector (DV) packet */
typedef struct {
    uint32_t dest_ip;        // Destination IP address
    uint32_t mask;           // Network mask
    uint32_t distance;       // Distance metric
} __attribute__((packed)) dv_packet_t;

/* Ethernet type for DV packets */
#define ETH_TYPE_DV 0x88b5   // Unused ethernet type for DV protocol

 

#endif // COMMON_H