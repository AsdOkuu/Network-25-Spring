
#include <stdio.h>
#include <pcap.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

/*********************************************************************************
 * common data structures and function prototypes 
*********************************************************************************/

#define MTU 1000

typedef struct Ethernet_Header
{
    u_int8_t  dhost[6]; // Destination MAC address
    u_int8_t  shost[6]; // Source MAC address
    u_int16_t type;     // Ethernet type
} ethernet_header_t;    // 14 bytes 


typedef struct __attribute__((packed)) My_Header
{
    uint32_t seq_num;
    uint8_t ack_flag;
    uint8_t pad;
} my_header_t;          //  6bytes


/*********************************************************************************
 * common initialization in both sender and receiver 
*********************************************************************************/

char nic[10] = "";
pcap_t *handle = NULL;

int init_nic(){
    char err_buf[PCAP_ERRBUF_SIZE];
    handle = pcap_open_live(nic, 65536, 10, 1, err_buf);
    if (handle==NULL){
        printf( "Cannot open device %s: %s\n", nic, err_buf);
        return 0;
    }
    return 0;
}

int close_nic(){
    pcap_close(handle);
    return 0;
}

/*********************************************************************************
 * sender 
*********************************************************************************/

#define DEST_MAC "\xaa\xbb\xcc\x00\x00\x01" // Destination MAC address
#define SRC_MAC  "\xaa\xbb\xcc\x00\x00\x02" // Source MAC address

#define NUM_PKT 10

int send_pkt(const u_char *pkt, int len){
    printf("=====================\n");
    printf("Sent a packet\n");
    printf("=====================\n");
    fflush(stdout);
    //int ret = pcap_inject(handle, pkt, len);
    int ret = pcap_sendpacket(handle, pkt, len);
    if(ret==-1){
	    printf("Error Sending Packet: %s\n", pcap_geterr(handle)); fflush(stdout);
    }
}

int run_sender(){
    //printf("run sender\n"); fflush(stdout);
    // prepare NUM_PKT packets to send
    u_char packet[NUM_PKT][MTU];
    for(int i=0; i<NUM_PKT; i++){
        ethernet_header_t *ether_header;
        my_header_t *my_header;
        u_char *payload;
        // prepare IP header
        ether_header = (ethernet_header_t*) packet[i];
        memcpy(ether_header->dhost, DEST_MAC, 6);
        memcpy(ether_header->shost, SRC_MAC, 6);
        ether_header->type=0xAAAA;
        // prepare my header
        my_header = (my_header_t *)(packet[i] + sizeof(ethernet_header_t));
        my_header->seq_num = i;
        my_header->ack_flag = 0;
        // prepare payload
        payload = packet[i] + sizeof(ethernet_header_t) + sizeof(my_header_t);
        for(u_char *p = payload; p < (u_char*) (packet[i] + MTU); p++) {
        	*p = 'a'+i;
    	}
    }
    // sends them out
    for(int i=0; i<NUM_PKT; i++){
        send_pkt(packet[i], MTU);
    }
}


/*********************************************************************************
 * receiver  
*********************************************************************************/

void recv_handler(u_char *user_data, const struct pcap_pkthdr *head, const u_char *pkt_data) {
    printf("=====================\n");
    printf("Receive a packet\n");
    // print user
    printf("-------- user -----------\n");
    printf("User: %s\n", user_data);
    // print head
    printf("-------- head -----------\n");
    time_t rawtime = head->ts.tv_sec;
    struct tm *timeinfo;
    char buffer[80];
    timeinfo=localtime(&rawtime);
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("Captured Time: %s.%06ld\n", buffer, head->ts.tv_usec);
    printf("Captured Length: %u\n", head->caplen);
    printf("Raw Packet Length: %u\n", head->len);
    printf("--------- pkt_data ------\n");
    ethernet_header_t *ether_header = (ethernet_header_t *)pkt_data;
    my_header_t *my_header = (my_header_t *)(pkt_data + sizeof(ethernet_header_t));
    u_char *payload = (u_char *)(my_header + sizeof(my_header_t));
    printf("Ethernet DST MAC: %02X:%02X:%02X:%02X:%02X:%02X \n",
            ether_header->dhost[0], ether_header->dhost[1], ether_header->dhost[2],
            ether_header->dhost[3], ether_header->dhost[4], ether_header->dhost[5]);
    printf("Ethernet SRC MAC: %02X:%02X:%02X:%02X:%02X:%02X \n",
            ether_header->shost[0], ether_header->shost[1], ether_header->shost[2],
            ether_header->shost[3], ether_header->shost[4], ether_header->shost[5]);
    printf("Ethernet Type: %04X \n", ether_header->type);
    printf("MyHeader Sequence Number: %u \n", my_header->seq_num );
    printf("MyHeader ACK: %u \n", my_header->ack_flag);
    fflush(stdout);
}

void* run_receiver(void *args){
    // start a thread and close after 4 seconds
    printf("start receiver thread\n"); fflush(stdout);
    pcap_loop(handle, 0, recv_handler, "Demo");
    printf("pcap_loop is broken\n"); fflush(stdout);
}

int main (int argc, char** argv) {
    if(argc != 3){ 
        printf("usage: %s sender/receiver NIC\n", argv[0]);
        return 0;
    }
    strcpy(nic, argv[2]);
    init_nic();
    if(strcmp(argv[1], "sender")==0){
        sleep(3);
        run_sender();
    }
    else{
        pthread_t t;
        void *res;
        pthread_create(&t, NULL, run_receiver, NULL);
        sleep(5);
	    pcap_breakloop(handle);
        //pthread_cancel(t);
        pthread_join(t, NULL);
        //if(res==PTHREAD_CANCELED){
            printf("receiver thread terminated\n");
        //}
        //else{
        //    printf("receiver thread not terminated\n");
        //}
    }
    close_nic();
    return 0;
}
