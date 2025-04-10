#include <sys/types.h>
#include <sys/time.h>
#include <pcap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>

#define LOG 


/* Global variables */
// common
net_device_t devices[MAX_DEVICES];
int device_count = 0;
packet_buffer_t pkt_buffer;

// switch
forwarding_db_t fdb;  // Forwarding database

// router
#define MAX_ROUTES 32
#define DV_INTERVAL 1000000  // 1 second
#define DV_SLEEP 500000      // 0.5 second

/* Routing table */
typedef struct {
    route_entry_t entries[MAX_ROUTES];
    int count;
} routing_table_t;

/* Control plane message buffer */
typedef struct {
    packet_entry_t packets[MAX_PACKETS];
    int head;
    int tail;
    pthread_mutex_t lock;
} cp_buffer_t;

routing_table_t rt;  // Routing table
cp_buffer_t cp_buf;  // Control plane message buffer

/* Forward declarations */
void *control_plane_thread(void *arg);
int load_rules_from_file(const char *filename);


/************************************
 * Helper functions
 ***********************************/
 
void print_mac(const uint8_t *mac) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
void print_ip(uint32_t ip) {
    printf("%u.%u.%u.%u\n", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
}


/************************************
 *  common
 ***********************************/



int common_init(){
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *alldevs;
    
    // Initialize packet buffer
    memset(&pkt_buffer, 0, sizeof(pkt_buffer));
    pkt_buffer.head = pkt_buffer.tail = 0;
    pthread_mutex_init(&pkt_buffer.lock, NULL);

    // Find all network devices
    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        fprintf(stderr, "Error finding devices: %s\n", errbuf);
        return -1;
    }

    // Filter and store switch/host devices
    pcap_if_t *d;
    for (d = alldevs; d != NULL && device_count < MAX_DEVICES; d = d->next) {
        if (strncmp(d->name, "device", 6) == 0 ||
            strncmp(d->name, "host", 4) == 0) {
            
            strncpy(devices[device_count].name, d->name, 31);
            devices[device_count].index = device_count;
            
            // Open device for capture
            devices[device_count].handle = pcap_open_live(d->name,
                PACKET_BUF_SIZE, 1, 1000, errbuf);
            if (!devices[device_count].handle) {
                fprintf(stderr, "Couldn't open device %s: %s\n",
                    d->name, errbuf);
                continue;
            }

            // Create capture thread
            if (pthread_create(&devices[device_count].thread_id,
                NULL, capture_thread, &devices[device_count]) != 0) {
                fprintf(stderr, "Failed to create thread for %s\n", d->name);
                pcap_close(devices[device_count].handle);
                continue;
            }

            device_count++;
        }
    }

    pcap_freealldevs(alldevs);
    printf("Initialized %d network devices\n", device_count);
    return device_count > 0 ? 0 : -1;
}



/************************************
 *  hub
 ***********************************/

int send_packet(net_device_t *dev, const uint8_t *data, uint32_t len) {
    if (pcap_inject(dev->handle, data, len) == -1) {
        fprintf(stderr, "Error sending packet on %s: %s\n",
               dev->name, pcap_geterr(dev->handle));
        return -1;
    }
    return 0;
}

void Hub() {
    common_init();
    while (1) {
        pthread_mutex_lock(&pkt_buffer.lock);
        
        // Check if buffer is empty
        if (pkt_buffer.head == pkt_buffer.tail) {
            pthread_mutex_unlock(&pkt_buffer.lock);
            usleep(1000); // Sleep 1ms if buffer is empty
            continue;
        }
        
        // Get packet from buffer
        packet_entry_t *entry = &pkt_buffer.packets[pkt_buffer.tail];
        pkt_buffer.tail = (pkt_buffer.tail + 1) % MAX_PACKETS;
        pthread_mutex_unlock(&pkt_buffer.lock);
        
        // Forward packet to all other devices
        /*************************************
         * start of your code
         *************************************/
        for(int i = 0; i < device_count; i++) {
            if(&devices[i] == entry->device) continue;
            send_packet(&devices[i], entry->data, entry->len);
        }
        /*************************************
         * end of your code
         *************************************/

    }
}


/************************************
 *  switch
 ***********************************/



void Switch(){
    common_init();
    memset(&fdb, 0, sizeof(fdb));  // Initialize forwarding database

    while (1) {
        pthread_mutex_lock(&pkt_buffer.lock);
        
        // Check if buffer is empty
        if (pkt_buffer.head == pkt_buffer.tail) {
            pthread_mutex_unlock(&pkt_buffer.lock);
            usleep(1000); // Sleep 1ms if buffer is empty
            continue;
        }
        
        // Get packet from buffer
        packet_entry_t *entry = &pkt_buffer.packets[pkt_buffer.tail];
        pkt_buffer.tail = (pkt_buffer.tail + 1) % MAX_PACKETS;
        pthread_mutex_unlock(&pkt_buffer.lock);
        

        /*************************************
         * start of your code
         *************************************/
         
        // Get Ethernet header

        
        // Learn source MAC address

        
        // Forward packet
    
            // case 1: Found destination, forward to that device

                
            // case 2: Flood to all ports except ingress
        
        /*************************************
         * end of your code
         *************************************/
        
    }
}

 /************************************
 *  router
 ***********************************/

void Router(){
    // Initialize common components
    if (common_init() != 0) {
        fprintf(stderr, "Failed to initialize router\n");
        return;
    }

    // Initialize routing table and control plane buffer
    memset(&rt, 0, sizeof(rt));
    memset(&cp_buf, 0, sizeof(cp_buf));
    pthread_mutex_init(&cp_buf.lock, NULL);

    // Load rules from file if exists
    if (load_rules_from_file("rules.txt") != 0) {
        fprintf(stderr, "No rules.txt found or error loading rules\n");
    }

    // Start control plane thread
    pthread_t cp_thread;
    if (pthread_create(&cp_thread, NULL, control_plane_thread, NULL) != 0) {
        fprintf(stderr, "Failed to create control plane thread\n");
        return;
    }

    usleep(DV_SLEEP); // Ensure control plane thread starts

    // Main router loop
    while (1) {
        pthread_mutex_lock(&pkt_buffer.lock);
        
        // Check if buffer is empty
        if (pkt_buffer.head == pkt_buffer.tail) {
            pthread_mutex_unlock(&pkt_buffer.lock);
            usleep(1000); // Sleep 1ms if buffer is empty
            continue;
        }
        
        // Get packet from buffer
        packet_entry_t *entry = &pkt_buffer.packets[pkt_buffer.tail];
        pkt_buffer.tail = (pkt_buffer.tail + 1) % MAX_PACKETS;
        pthread_mutex_unlock(&pkt_buffer.lock);

        // Check if it's a DV packet
        eth_header_t *eth = (eth_header_t *)entry->data;
        if (ntohs(eth->ether_type) == ETH_TYPE_DV) {
            // Add to control plane buffer
            pthread_mutex_lock(&cp_buf.lock);
            if ((cp_buf.head + 1) % MAX_PACKETS != cp_buf.tail) {
                memcpy(&cp_buf.packets[cp_buf.head], entry, sizeof(packet_entry_t));
                cp_buf.head = (cp_buf.head + 1) % MAX_PACKETS;
            }
            pthread_mutex_unlock(&cp_buf.lock);
            continue;
        }


        /*********************************
         * start of your code
         ********************************/

        // Forward packet based on routing table       

        /*********************************
         * end of your code
         ********************************/
    }
}

int load_rules_from_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;

    char line[256];
    while (fgets(line, sizeof(line), fp) && rt.count < MAX_ROUTES) {
        char network[32], out_dev[32];
        unsigned int prefix_len, distance;
        
        if (sscanf(line, "%s %s %u", network, out_dev, &distance) == 3) {
            // Parse CIDR notation
            uint32_t ip = 0, mask = 0;
            char *slash = strchr(network, '/');
            if (slash) {
                *slash = '\0';
                prefix_len = atoi(slash + 1);
                if (prefix_len > 32) continue;
                
                // Convert IP string to number
                struct in_addr addr;
                if (inet_pton(AF_INET, network, &addr) != 1) continue;
                ip = ntohl(addr.s_addr);
                
                // Create mask from prefix length
                mask = prefix_len ? ~((1 << (32 - prefix_len)) - 1) : 0;
                
                // Add to routing table
                rt.entries[rt.count].dest_ip = ip & mask;
                rt.entries[rt.count].mask = mask;
                strncpy(rt.entries[rt.count].out_dev, out_dev, 31);
                rt.entries[rt.count].distance = distance;
                rt.count++;
            }
        }
    }
    fclose(fp);
    return 0;
}

void *control_plane_thread(void *arg) {
    (void)arg; // Explicitly mark as unused
    uint64_t last_broadcast = 0;
    
    while (1) {
        // Process control plane messages
        pthread_mutex_lock(&cp_buf.lock);
        if (cp_buf.head != cp_buf.tail) {
            packet_entry_t *entry = &cp_buf.packets[cp_buf.tail];
            cp_buf.tail = (cp_buf.tail + 1) % MAX_PACKETS;
            pthread_mutex_unlock(&cp_buf.lock);
            
            /*********************************
            * start of your code
            ********************************/

            // Process DV packet and update routing table
            
            // Check if destination exists in routing table
            
            // Add new entry if destination not found and table not full

            /*********************************
            * end of your code
            ********************************/

        } else {
            pthread_mutex_unlock(&cp_buf.lock);
            usleep(DV_SLEEP);
        }
        
        /*********************************
        * start of your code
        ********************************/

        // Broadcast routing table periodically
        uint64_t now = get_current_time_us();
        if (now - last_broadcast >= DV_INTERVAL) {
            last_broadcast = now;

            // Iterate through routing table 
                // Iterate through devices
                        // Create DV packet (Tips: MAC address can be arbitrary)
                        // Send DV packet to all devices 

        }
        /*********************************
        * start of your code
        ********************************/
    }
    return NULL;
}

uint64_t get_current_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

/************************************
 *  Packet capture thread
 ***********************************/

void *capture_thread(void *arg) {
    net_device_t *dev = (net_device_t *)arg;
    struct pcap_pkthdr header;
    const u_char *packet;
    
    printf("Starting capture on %s\n", dev->name);
    
    while (1) {
        packet = pcap_next(dev->handle, &header);
        if (!packet) continue;        
        
        pthread_mutex_lock(&pkt_buffer.lock);
        
        // Check if buffer is full
        if ((pkt_buffer.head + 1) % MAX_PACKETS == pkt_buffer.tail) {
            fprintf(stderr, "Packet buffer full, dropping packet\n");
            pthread_mutex_unlock(&pkt_buffer.lock);
            continue;
        }
        
        // Store packet in buffer

        packet_entry_t *entry = &pkt_buffer.packets[pkt_buffer.head];
        entry->device = dev;
        entry->len = header.len;
        entry->timestamp = header.ts.tv_sec * 1000000 + header.ts.tv_usec;
        memcpy(entry->data, packet, header.len > PACKET_BUF_SIZE ? PACKET_BUF_SIZE : header.len);
        
        pkt_buffer.head = (pkt_buffer.head + 1) % MAX_PACKETS;
        pthread_mutex_unlock(&pkt_buffer.lock);
    }
    
    return NULL;
}

