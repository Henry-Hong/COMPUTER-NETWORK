#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void error_handling(char* message);

int main(int argc, char* argv[])
{
    int serv_sock;
    int clnt_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    char message[] = "";

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    // 1단계: socket() 소켓 생성
    // socket(domain, type, protocol)
    serv_sock = socket(PF_INET, SOCK_STREAM, 0); // PF_INET: IPv4 인터넷 프로토콜,` SOCK_STREAM: TCP 프로토콜
    if (serv_sock == -1)
        error_handling("socket() error");
    else
        printf("socket success\n");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // IPv4 인터넷 프로토콜
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 32비트 주소 할당 (소켓이 동작하는 컴퓨터의 IP 주소가 자동으로 할당)
    serv_addr.sin_port = htons(atoi(argv[1])); // 16비트 TCP/UDP 포트 번호

    // 2단계: bind() 주소 연결
    // bind(socket, address, address_len)
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    else
        printf("bind success\n");

    // 2.5단계: listen() 연결 요청 대기열 생성
    // listen(socket, backlog)
    // backlog: 동시에 처리할 수 있는 연결 요청의 최대 개수
    if (listen(serv_sock, 2) == -1)
        error_handling("listen() error");
    else
        printf("listen success\n");

    // 3단계: accept()
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");
    else
        printf("accept success\n");

    // 4단계: 클라이언트로 메시지 전송
    write(clnt_sock, message, sizeof(message));

    close(clnt_sock);
    close(serv_sock);

    return 0;
}

void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
