#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>

#define CONNECTIONS_FILE "connections.txt"

#define LOG

typedef struct {
    int conn_id;
    char local_ip[16];  // 点分十进制字符串格式
    uint16_t local_port;
    char remote_ip[16]; // 点分十进制字符串格式
    uint16_t remote_port;
    char filename[256];
    size_t file_len;
    size_t received_len;
    void* buffer;
    pthread_t thread;
    double throughput;
} connection_t;

void* sender_thread(void* arg) {
    connection_t* conn = (connection_t*)arg;
    struct timeval start, end;

    // sleep for 1 second to allow receiver to set up
    sleep(1);
    
    gettimeofday(&start, NULL);
    m_send(conn->conn_id, conn->buffer, conn->file_len);
    gettimeofday(&end, NULL);
    
    double elapsed = (end.tv_sec - start.tv_sec) +
                   (end.tv_usec - start.tv_usec) / 1000000.0;
    conn->throughput = (conn->file_len * 8) / (elapsed * 1000000);
        
    printf("Sender throughput: %.2f Mbps\n", conn->throughput);

    return NULL;
}

void* receiver_thread(void* arg) {
    connection_t* conn = (connection_t*)arg;
    struct timeval start, end;
    
    gettimeofday(&start, NULL);
    size_t received = m_recv(conn->conn_id, conn->buffer, conn->file_len);
    gettimeofday(&end, NULL);
    
    if (received > 0) {
        conn->received_len = received;
    }
    
    // 返回接收到的数据长度
    return NULL;
}

int main(int argc, char* argv[]) {

    if (argc != 2 || (strcmp(argv[1], "sender") != 0 && strcmp(argv[1], "receiver") != 0)) {
        fprintf(stderr, "Usage: %s <sender|receiver>\n", argv[0]);
        return 1;
    }

    
    int is_sender = strcmp(argv[1], "sender") == 0;
    FILE* fp = fopen(CONNECTIONS_FILE, "r");
    if (!fp) {
        perror("Failed to open connections file");
        return 1;
    }

    
    connection_t connections[CCB_TABLE_SIZE];
    int conn_count = 0;
    char line[1024];
    
    // 解析connections.txt
    while (fgets(line, sizeof(line), fp) && conn_count < CCB_TABLE_SIZE) {
        connection_t* conn = &connections[conn_count];
        memset(conn, 0, sizeof(connection_t));
        
        if (sscanf(line, "%15s %hu %15s %hu %255s %zu",
                    conn->local_ip, &conn->local_port,
                    conn->remote_ip, &conn->remote_port,
                    conn->filename, &conn->file_len) != 6) {
            fprintf(stderr, "Invalid line format: %s\n", line);
            continue;
        }
        
        
        // 初始化连接
        conn->conn_id = init_connection(conn->local_ip, conn->local_port,
                                      conn->remote_ip, conn->remote_port);
        if (conn->conn_id < 0) {
            fprintf(stderr, "Failed to init connection for %s\n", conn->filename);
            continue;
        }

        
        // 分配缓冲区
        conn->buffer = malloc(conn->file_len);
        if (!conn->buffer) {
            perror("Failed to allocate buffer");
            continue;
        }


        // 如果是sender，读取文件内容
        if (is_sender) {
            FILE* file = fopen(conn->filename, "rb");
            if (!file || fread(conn->buffer, 1, conn->file_len, file) != conn->file_len) {
                perror("Failed to read file");
                free(conn->buffer);
                continue;
            }
            fclose(file);
        }

        
        // 启动线程
        pthread_create(&conn->thread, NULL,
                     is_sender ? sender_thread : receiver_thread,
                     conn);
        conn_count++;
    }
    fclose(fp);
    
    // 等待所有线程完成
    for (int i = 0; i < conn_count; i++) {
        pthread_join(connections[i].thread, NULL);
    }

    // 如果是receiver，将每个连接的缓冲区数据写入文件
    if (!is_sender) {
        for (int i = 0; i < conn_count; i++) {
            FILE* fp = fopen(connections[i].filename, "wb");
            if (fp) {
                fwrite(connections[i].buffer, 1, connections[i].file_len, fp);
                fclose(fp);
            }
        }
    }

    // 释放所有缓冲区
    for (int i = 0; i < conn_count; i++) {
        free(connections[i].buffer);
    }
    
    return 0;
}