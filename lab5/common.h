#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <netinet/in.h>
#include <sys/time.h>

// 宏定义
#define MTP_MAX_MSG_LEN (32*1024*1024)  // 消息最大长度32MB
#define MTP_PAYLOAD_LEN 128              // 载荷长度128B
#define MTP_MAX_SEGMENTS (MTP_MAX_MSG_LEN/MTP_PAYLOAD_LEN) // 最大分组数量
#define CCB_TABLE_SIZE 10                // 连接表大小

// 消息传输协议首部
typedef struct {
    uint32_t seq_num;     // 序列号
    uint16_t ack_flag;    // ACK标志位
    uint16_t payload_len; // 有效载荷长度
} __attribute__((packed)) mtp_header_t;

// 连接控制块
typedef struct {
    // 连接信息
    char local_ip[16];  // 点分十进制字符串格式
    uint16_t local_port;
    char remote_ip[16]; // 点分十进制字符串格式
    uint16_t remote_port;

    // 发送相关
    int udp_sock;                      // UDP socket描述符
    struct sockaddr_in remote_addr;    // 远端地址
    uint32_t window_base;              // 窗口后沿
    double window_size;              // 窗口长度
    uint32_t max_seq;                  // 最大序列号
    uint8_t congestion_state;          // 0=慢启动 1=拥塞避免
    struct timeval last_congestion;    // 上次拥塞时间
    
    // 重传和RTT估计
    struct timeval timestamps[MTP_MAX_SEGMENTS]; // 时间戳数组
    uint8_t acks_received[MTP_MAX_SEGMENTS];     // ACK数组
    uint32_t retrans_records[MTP_MAX_SEGMENTS];  // 重传记录数组
    double rtt_estimated;              // RTT估计值
    double rtt_variation;              // RTT方差

    // 接收相关  
    uint8_t recv_records[MTP_MAX_SEGMENTS];      // 接收记录数组
    uint32_t recv_progress;            // 接收进度
    struct timeval recv_complete_time; // 接收完成时间戳
} ccb_t;

// 连接查询表
typedef struct {
    ccb_t entries[CCB_TABLE_SIZE];  // 固定大小的CCB数组
    int count;                // 当前连接数
} ccb_table_t;

// 函数声明
int init_connection(const char* local_ip, uint16_t local_port,
                   const char* remote_ip, uint16_t remote_port);
void close_connection(int conn_id);
int m_send(int conn_id, const void* buf, size_t len);
int m_recv(int conn_id, void* buf, size_t len);

#endif // COMMON_H