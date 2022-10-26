// 2020118016 홍희림
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

enum {
    REQUEST = 0,
    RESPONSE = 1,
    QUIT = 2,
    ERROR = 0,
    SUCCESS = 1
};

// packet
typedef struct {
    int cmd; //   command (0: request, 1: response, 2: quit)
    char addr[20]; // dotted-decimal address!
    struct in_addr iaddr; // inet_aton() 의 반환값이 들어가는곳
    int result; // 0: Error, 1: Success
} PACKET;

void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char* argv[])
{
    //명령형 인자 에러 처리
    if (argc != 3)
        error_handling("Usage : ./hw2_client <IP> <port>");

    int sock;
    PACKET packet;
    struct sockaddr_in serv_addr;

    // create socket
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // 서버연결을 위한 주소체계 (IPv4)
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); // 서버의 IP 주소
    serv_addr.sin_port = htons(atoi(argv[2])); // 서버의 PORT 넘버

    // 소켓 연결 요청!
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connection error!");

    // 패킷 송수신
    while (1) {
        memset(&packet, 0, sizeof(packet));
        packet.cmd = REQUEST;
        printf("Input dotted-decimal address (Q: quit): ");
        scanf("%s", packet.addr);

        // 소켓 종료 로직
        if (!strcmp(packet.addr, "Q")) {
            packet.cmd = QUIT;
            if (write(sock, (PACKET*)&packet, sizeof(packet)) == -1)
                error_handling("wrtie() error!");

            printf("[Tx] cmd: 2(QUIT)\n");
            printf("Client socket close and exit\n");
            close(sock);
            return 0;
        }

        // 패킷 송신
        if (write(sock, (PACKET*)&packet, sizeof(packet)) == -1)
            error_handling("write() error");
        else
            printf("[Tx] cmd: %d, addr: %s\n", packet.cmd, packet.addr);

        // 패킷 수신
        if (read(sock, (PACKET*)&packet, sizeof(packet)) == -1)
            error_handling("read() error");

        if (packet.result == SUCCESS)
            printf("[Rx] cmd: %d, Address conversion: %#x (result: %d)\n", packet.cmd, packet.iaddr.s_addr, packet.result);
        else
            printf("[Rx] cmd: %d, Address conversion fail:(result: %d)\n", packet.cmd, packet.result);
        printf("\n\n");
    }

    close(sock);
    return 0;
}
