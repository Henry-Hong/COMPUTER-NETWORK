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
    int serv_sock;
    int clnt_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    Packet packet;
    int str_len;
    int write_len;
    int fd;
    int sent_len = 0;
    char file_name[30];

    // 입력값 에러 처리
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    printf("-------------------------------\n");
    printf("   File Transmission Server\n");
    printf("-------------------------------\n");

    // 1. 소켓 생성 - TCP
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        error_handling("socket() error");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    // 2. 소켓 바인드 - 포트번호 할당
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == 1) {
        error_handling("bind() error");
    }

    // 3. 소켓 리스닝 - 연결 요청 대기
    if (listen(serv_sock, 5) == 1) {
        error_handling("listen() error");
    }

    // 4. 소켓 허가 - 연결 요청 수락
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1) {
        error_handling("accept() error");
    }

    // 5. 파일 이름 수신
    str_len = read(clnt_sock, &packet, sizeof(packet));
    if (str_len == -1) {
        error_handling("read() error!");
    }

    // 6. 파일 열기 - 읽기 전용
    fd = open(packet.buf, O_RDONLY);

    // 7. 데이터 송수신
    if (fd == -1) {
        // 7-1. 찾고자 하는 file이 없을 경우
        printf("%s File Not Found\n", packet.buf);
        strcpy(packet.buf, "File Not Found");
        if (write(clnt_sock, &packet, sizeof(packet)) == -1)
            error_handling("write() error!");
        close(serv_sock);
        close(clnt_sock);
        return 0;
    }

    // 7-2. 찾고자하는 file이 있는 경우
    packet.seq = SEQ_START;
    strcpy(file_name, packet.buf);
    printf("[Server] sending %s\n", file_name);
    while (1) {
        // file read - 1번 파라미터에서 읽어서 2번 파라미에터에 대입
        packet.buf_len = read(fd, packet.buf, BUFFER_SIZE);
        sent_len += packet.buf_len;

        // socket write
        if (write(clnt_sock, &packet, sizeof(packet)) == -1)
            error_handling("write() error!");
        printf("[Server] Tx SEQ: %d, %d byte data\n", packet.seq, packet.buf_len);

        if (packet.buf_len == 0) {
            printf("No more to readin\n");
            break;
        }

        // socket read
        if (read(clnt_sock, &packet, sizeof(packet)) == -1)
            error_handling("read() error!");
        printf("[Server] Rx ACK: %d\n", packet.ack);

        // packet update
        packet.seq = packet.ack;
        printf("\n");
    }

    printf("%s sent (%d Bytes)\n", file_name, sent_len);

    close(clnt_sock);
    close(serv_sock);

    return 0;
}