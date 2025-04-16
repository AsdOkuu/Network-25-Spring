#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PAYLOAD_LEN 1024  // 定义载荷长度

// 自定义首部结构体
typedef struct {
    uint32_t seq_num;     // 序列号
    uint32_t length;      // 长度
} CustomHeader;

void print_usage(const char *prog_name) {
    printf("Usage: %s <server|client> <ip> <port>\n", prog_name);
}

int run_server(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    char buffer[PAYLOAD_LEN + sizeof(CustomHeader)];
    
    // 创建UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    // 绑定socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return -1;
    }

    printf("Server listening on %s:%d\n", ip, port);

    while (1) {
        // 接收数据
        ssize_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                   (struct sockaddr *)&cli_addr, &cli_len);
        if (recv_len < 0) {
            perror("recvfrom failed");
            continue;
        }

        // 解析自定义首部
        CustomHeader *header = (CustomHeader *)buffer;
        char *payload = buffer + sizeof(CustomHeader);

        // 打印接收到的信息
        printf("Received from %s:%d\n", 
               inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        printf("Custom Header - Seq: %u, Length: %u; ", 
               ntohl(header->seq_num), ntohl(header->length));
        printf("First 10 bytes of payload: ");
        for (int i = 0; i < 10 && i < (recv_len - sizeof(CustomHeader)); i++) {
            printf("%02x ", payload[i]);
        }
        printf("\n");
        fflush(stdout);
    }

    close(sockfd);
    return 0;
}

int run_client(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[PAYLOAD_LEN + sizeof(CustomHeader)];

    // 创建UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    // 发送10个分组
    for (uint32_t seq = 0; seq < 10; seq++) {
        CustomHeader *header = (CustomHeader *)buffer;
        char *payload = buffer + sizeof(CustomHeader);

        // 填充自定义首部
        header->seq_num = htonl(seq);
        header->length = htonl(PAYLOAD_LEN);

        // 填充载荷
        char fill_char = 'a' + seq;
        memset(payload, fill_char, PAYLOAD_LEN);

        // 发送数据
        if (sendto(sockfd, buffer, sizeof(CustomHeader) + PAYLOAD_LEN, 0,
                  (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("sendto failed");
            close(sockfd);
            return -1;
        }

        printf("Sent packet with seq=%u, fill_char=%c\n", seq, fill_char);
        usleep(100000);  // 间隔100ms
    }

    close(sockfd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        print_usage(argv[0]);
        return -1;
    }

    const char *mode = argv[1];
    const char *ip = argv[2];
    int port = atoi(argv[3]);

    if (strcmp(mode, "server") == 0) {
        return run_server(ip, port);
    } else if (strcmp(mode, "client") == 0) {
        return run_client(ip, port);
    } else {
        print_usage(argv[0]);
        return -1;
    }
}