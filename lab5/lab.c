#include "common.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <fcntl.h>

#define LOG



static ccb_table_t conn_table = {0};

int init_connection(const char* local_ip, uint16_t local_port,
                   const char* remote_ip, uint16_t remote_port) {
    if (conn_table.count >= CCB_TABLE_SIZE) {
        return -1;
    }

    ccb_t* ccb = &conn_table.entries[conn_table.count];
    memset(ccb, 0, sizeof(ccb_t));

    // 设置连接信息
    strncpy(ccb->local_ip, local_ip, sizeof(ccb->local_ip)-1);
    ccb->local_ip[sizeof(ccb->local_ip)-1] = '\0';
    ccb->local_port = local_port;
    strncpy(ccb->remote_ip, remote_ip, sizeof(ccb->remote_ip)-1);
    ccb->remote_ip[sizeof(ccb->remote_ip)-1] = '\0';
    ccb->remote_port = remote_port;

    // 初始化窗口参数
    ccb->window_base = 0;
    ccb->window_size = 1;
    ccb->rtt_estimated = 0.2; // 200ms
    ccb->rtt_variation = 0;
    ccb->recv_progress = 0;
    memset(&ccb->recv_complete_time, 0, sizeof(struct timeval));


    /***********************
     * start of your code
     **********************/

    // 创建UDP socket
    if ((ccb->udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }

    // 绑定本地地址
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(ccb->local_ip);
    local_addr.sin_port = htons(ccb->local_port);
    
    if (bind(ccb->udp_sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind failed");
        close(ccb->udp_sock);
        return -1;
    }

    // 设置远端地址
    memset(&ccb->remote_addr, 0, sizeof(ccb->remote_addr));
    ccb->remote_addr.sin_family = AF_INET;
    ccb->remote_addr.sin_addr.s_addr = inet_addr(ccb->remote_ip);
    ccb->remote_addr.sin_port = htons(ccb->remote_port);

     /***********************
     * end of your code
     **********************/
    

    
    return conn_table.count++;
}

void close_connection(int conn_id) {
    if (conn_id < 0 || conn_id >= conn_table.count) {
        return;
    }

    ccb_t* ccb = &conn_table.entries[conn_id];
    if (ccb->udp_sock >= 0) {
        close(ccb->udp_sock);
    }
    memset(ccb, 0, sizeof(ccb_t));
}

// 更新RTT估计
static void update_rtt(ccb_t* ccb, double rtt_sample) {
    /***********************
     * start of your code
     **********************/

    /***********************
     * end of your code
     **********************/
}

// 拥塞控制
static void congestion_control(ccb_t* ccb, int is_timeout) {
    struct timeval now;
    gettimeofday(&now, NULL);
    
    /***********************
     * start of your code
     **********************/

     if (is_timeout) {
        // 检查是否在保护期内，如果不在
            // 超时处理
            // 窗口减半，不小于1
            // 进入拥塞避免
            // 记录保护期开始
    } else {
        // ACK处理
            // 慢启动阶段：每个ACK窗口+1
            // 拥塞避免阶段：每个ACK窗口+1/window_size
    }

    /***********************
     * end of your code
     **********************/

}

int m_send(int conn_id, const void* buf, size_t len) {
    if (conn_id < 0 || conn_id >= conn_table.count) {
        return -1;
    }

    ccb_t* ccb = &conn_table.entries[conn_id];
    
    // 检查缓冲区长度
    if (len > MTP_MAX_MSG_LEN) {
        fprintf(stderr, "Message too large\n");
        return -1;
    }

    // 初始化数组
    memset(ccb->timestamps, 0, sizeof(struct timeval) * MTP_MAX_SEGMENTS);
    memset(ccb->acks_received, 0, sizeof(uint8_t) * MTP_MAX_SEGMENTS);
    memset(ccb->retrans_records, 0, sizeof(uint32_t) * MTP_MAX_SEGMENTS);

    // 计算最大序列号
    uint32_t max_seq = (len + MTP_PAYLOAD_LEN - 1) / MTP_PAYLOAD_LEN;
    ccb->max_seq = max_seq;

    // 设置非阻塞模式
    int flags = fcntl(ccb->udp_sock, F_GETFL, 0);
    fcntl(ccb->udp_sock, F_SETFL, flags | O_NONBLOCK);


    // 发送循环
    while (1) {

        /***********************
         * start of your code
         **********************/

        // 处理接收分组
            // 解析头部
            // 更新ACK数组
                // 仅对非重传分组更新RTT
                // 拥塞控制
        char local_buf[MTP_PAYLOAD_LEN + sizeof(mtp_header_t) + 8];
        ssize_t recv_len = recvfrom(ccb->udp_sock, local_buf, sizeof(local_buf), 0, NULL, NULL);
        mtp_header_t *header = (mtp_header_t*)local_buf;
        if(recv_len > 0) {
            if(!header->ack_flag) {
                fprintf(stderr, "Received non-ACK packet, seq_num: %u, payload_len: %u\n",
                        ntohl(header->seq_num), ntohs(header->payload_len));
                return -1;
            }
            uint32_t seq_num = ntohl(header->seq_num);
            ccb->acks_received[seq_num] = 1;

            fprintf(stderr, "Conn %d: Received ACK packet, seq_num: %u\n", conn_id,
                    ntohl(header->seq_num));
        }
            
        // 滑动窗口
        // 发送分组
            // 检查超时
                    // 超时处理：拥塞控制，记录重传
            // 超时或者第一次发送：构造并发送分组
            // 记录发送时间
        // 检查完成度
        while(ccb->window_base + ccb->window_size < ccb->max_seq
        && ccb->acks_received[ccb->window_base] == 1) {
            ccb->window_base++;
        }

        int complete = 1;
        for(uint32_t i = ccb->window_base; i < ccb->window_base + ccb->window_size && i < ccb->max_seq; i++) {
            if (ccb->acks_received[i] == 0) {
                // 检查超时
                struct timeval now;
                gettimeofday(&now, NULL);
                double elapsed = (now.tv_sec - ccb->timestamps[i].tv_sec) +
                                 ((double) now.tv_usec - ccb->timestamps[i].tv_usec) / 1e6;
                if ((ccb->timestamps[i].tv_sec == 0 && ccb->timestamps[i].tv_usec == 0) 
                || elapsed > ccb->rtt_estimated + 4 * ccb->rtt_variation) {
                    ccb->timestamps[i] = now;

                    mtp_header_t *header = (mtp_header_t*) local_buf;
                    int payload_len;
                    if(len > (i + 1) * MTP_PAYLOAD_LEN) {
                        payload_len = MTP_PAYLOAD_LEN;
                    } else {
                        payload_len = len - i * MTP_PAYLOAD_LEN;
                    }
                    header->seq_num = htonl(i);
                    header->ack_flag = 0;
                    header->payload_len = htons(payload_len);
                    memcpy(header + 1, buf + i * MTP_PAYLOAD_LEN, payload_len);
                    ssize_t sent_len = sendto(ccb->udp_sock, &local_buf, sizeof(local_buf), 0,
                                            (struct sockaddr*)&ccb->remote_addr, sizeof(ccb->remote_addr));
                    if (sent_len < 0) {
                        fprintf(stderr, "Sendto failed");
                        return -1;
                    }

                    fprintf(stderr, "Conn %d: Sent packet, seq_num: %u, payload_len: %u\n", conn_id,
                            ntohl(header->seq_num), ntohs(header->payload_len));
                }
            }
            if (ccb->acks_received[i] == 0) {
                complete = 0;
            }
        }

        if(complete) {
            fprintf(stderr, "All packets sent\n");
            break;
        }

        /***********************
         * end of your code
         **********************/
    }

    return 0;
}

// 构造ACK分组
static void build_ack_packet(mtp_header_t* header, uint32_t seq_num) {
    header->seq_num = seq_num;
    header->ack_flag = 1;
    header->payload_len = 0;
}

int m_recv(int conn_id, void* buf, size_t len) {
    if (conn_id < 0 || conn_id >= conn_table.count) {
        return -1;
    }

    ccb_t* ccb = &conn_table.entries[conn_id];
    
    // 检查缓冲区长度
    if (len > MTP_MAX_MSG_LEN) {
        fprintf(stderr, "Buffer too small for message\n");
        return -1;
    }

    // 初始化数组
    memset(ccb->recv_records, 0, sizeof(uint8_t) * MTP_MAX_SEGMENTS);
    ccb->recv_progress = 0;
    memset(&ccb->recv_complete_time, 0, sizeof(struct timeval));

    /***********************
     * start of your code
     **********************/

    // 计算最大序列号
    uint32_t max_seq = 0;
    max_seq = (len + MTP_PAYLOAD_LEN - 1) / MTP_PAYLOAD_LEN;
    
    // 设置非阻塞模式
    int flags = fcntl(ccb->udp_sock, F_GETFL, 0);
    fcntl(ccb->udp_sock, F_SETFL, flags | O_NONBLOCK);
    
    /***********************
     * end of your code
     **********************/


    
    // 接收循环
    while (1) {
        /***********************
         * start of your code
         **********************/

        
        // 处理接收分组
            // 解析头部
            // 写入接收缓冲区
            // 发送ACK
            // 更新接收记录
            // 更新接收进度
        char local_buf[MTP_PAYLOAD_LEN + sizeof(mtp_header_t) + 8];
        ssize_t recv_len = recvfrom(ccb->udp_sock, local_buf, sizeof(local_buf), 0, NULL, NULL);
        mtp_header_t *header = (mtp_header_t*)local_buf;
        if(recv_len > 0 && ccb->recv_records[ntohl(header->seq_num)] == 0) {
            if(header->ack_flag) {
                fprintf(stderr, "Received ACK packet");
                return -1;
            }
            uint32_t seq_num = ntohl(header->seq_num);
            uint16_t payload_len = ntohs(header->payload_len);
            memcpy(buf + seq_num * MTP_PAYLOAD_LEN, local_buf + sizeof(mtp_header_t), payload_len);
            ccb->recv_records[seq_num] = 1;
            ccb->recv_progress++;

            fprintf(stderr, "Conn %d: Received packet, seq_num: %u, payload_len: %u\n", conn_id,
                    ntohl(header->seq_num), ntohs(header->payload_len));

            build_ack_packet(header, header->seq_num);
            sendto(ccb->udp_sock, local_buf, sizeof(mtp_header_t), 0,
                   (struct sockaddr*)&ccb->remote_addr, sizeof(ccb->remote_addr));

            fprintf(stderr, "Conn %d: Sent ACK packet, seq_num: %u\n", conn_id,
                    ntohl(header->seq_num));
        }

        // 完成度检查
                // 首次完成，记录时间
                // 检查是否超过0.5秒
                    // 如果是，结束接收
        if(ccb->recv_progress >= max_seq) {
            if (ccb->recv_complete_time.tv_sec == 0 && ccb->recv_complete_time.tv_usec == 0) {
                gettimeofday(&ccb->recv_complete_time, NULL);
            } else {
                struct timeval now;
                gettimeofday(&now, NULL);
                double elapsed = (now.tv_sec - ccb->recv_complete_time.tv_sec) +
                                 ((double) now.tv_usec - ccb->recv_complete_time.tv_usec) / 1e6;
                if (elapsed > 0.5) {
                    break;
                }
            }
        }
        /***********************
         * end of your code
         **********************/
    }

    size_t received_len = ccb->recv_progress * MTP_PAYLOAD_LEN;
    if (ccb->recv_progress == max_seq) {
        received_len = received_len - (MTP_PAYLOAD_LEN - (received_len % MTP_PAYLOAD_LEN));
    }

    // 确保接收数据不超过缓冲区长度
    if (received_len > len) {
        return -1;
    }

    return received_len;
}
