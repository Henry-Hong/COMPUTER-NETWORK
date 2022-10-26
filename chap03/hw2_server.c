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
    if (argc != 2)
        error_handling("Usage : ./hw2_server <PORT>");

    printf("-------------------------\n");
    printf("Address Conversion Server\n");
    printf("-------------------------\n");

    int serv_sock;
    int clnt_sock;
    PACKET packet;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    // bind: 소켓에 주소 할당
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    // listen: 연결 요청 대기열 생성
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    // accept: 연결 요청 수락
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");

    // 연결된 클라이언트 소켓과 패킷 송수신

    long readSize = 0;
    while (1) {
        if ((readSize = read(clnt_sock, (PACKET*)&packet, sizeof(packet))) == -1)
            error_handling("read() error");

        // 패킷을 받았다면, 주소변환 처리 후 패킷을 보낸다.
        if (packet.cmd == REQUEST) {
            packet.cmd = RESPONSE; // 패킷 전송용으로 설정.
            printf("[Rx] Received Dotted-Decimal Address: %s\n", packet.addr);

            // 주소를 네트워크바이트 형식으로 변환 - 실패한경우
            if (!inet_aton(packet.addr, &packet.iaddr)) {
                packet.result = ERROR;
                printf("[Tx] Address conversion Fail: (%s)\n\n\n", packet.addr);
                if (write(clnt_sock, (PACKET*)&packet, sizeof(packet)) == -1)
                    error_handling("write() error!");
            }
            // 주소를 네트워크바이트 형식으로 변환 - 성공한경우
            else {
                packet.result = SUCCESS;
                printf("inet_aton(%s) -> %#x\n", packet.addr, packet.iaddr.s_addr);
                printf("[Tx] cmd: %d, iaddr: %#x, result: %d\n\n\n", packet.cmd, packet.iaddr.s_addr, packet.result);
                if (write(clnt_sock, (PACKET*)&packet, sizeof(packet)) == -1)
                    error_handling("write() error!");
            }
        } else if (packet.cmd == QUIT) {
            printf("[Rx] Quit message received!\n");
            printf("Server socket close and exit.\n");
            break;
        } else { // 클라이언트에서 강제로 연결을 그만뒀을때.
            error_handling("Unknown command");
            printf("readSize: %ld\n", readSize);
            // 버퍼에 남아있는? 그런건 어떻게 처리하지 ㅎ ㅠ
        }
    }

    close(clnt_sock);
    close(serv_sock);
    return 0;
}
