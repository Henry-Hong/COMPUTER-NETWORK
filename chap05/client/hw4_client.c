// 2020118016 홍희림
// TCP - SEQ, ACK 파일전송과정
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 100
#define SEQ_START 1000

typedef struct {
    int seq;
    int ack;
    int buf_len;
    char buf[BUFFER_SIZE];
} Packet;

void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char* argv[])
{
    int sock;
    struct sockaddr_in serv_addr;

    Packet packet;
    int fd; // file descripter
    long long recv_len = 0;
    char file_name[30];

    // 입력값 에러 처리
    if (argc != 3) {
        printf("Usage : %s <Ip> <port>\n", argv[0]);
        exit(1);
    }

    // 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // 연결 요청
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");

    // step1: 요청할 파일 이름 전송
    printf("Input file name: ");
    scanf("%s", file_name);

    memset(&packet, 0, sizeof(packet));
    strcpy(packet.buf, file_name);
    packet.buf_len = strlen(file_name);
    if (write(sock, &packet, sizeof(packet)) == -1)
        error_handling("write() error");
    else
        printf("[Client] request %s\n", packet.buf);

    // step2: 받아올 파일을 미리 생성
    // file 만들기
    fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == 1)
        error_handling("open() error!");

    // step3: 파일 수신(응답) 받기
    while (1) {
        if (read(sock, &packet, sizeof(packet)) == -1)
            error_handling("read() error");

        if (strcmp(packet.buf, "File Not Found") == 0) {
            printf("FILE NOT FOUND\n");
            close(sock);
            return 0;
        }

        if (packet.buf_len == 0) {
            printf("FILE TRANSFER COMPLETE\n");
            break;
        }

        printf("[Client] Rx SEQ: %d, len: %d bytes\n", packet.seq, packet.buf_len);
        recv_len += packet.buf_len;

        // write on file
        if (write(fd, packet.buf, packet.buf_len) == -1)
            error_handling("file write() error!");

        // packet update
        packet.ack = packet.seq + packet.buf_len + 1;

        // write on socket
        if (write(sock, &packet, sizeof(packet)) == -1)
            error_handling("socket write() error!");
        else
            printf("[Client] Tx ACK: %d\n", packet.ack);

        printf("\n");
    }

    printf("%s received (%lld Bytes)\n", file_name, recv_len);

    close(sock);

    return 0;
}
