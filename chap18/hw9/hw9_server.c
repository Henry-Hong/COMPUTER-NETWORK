#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
// import open
#include <fcntl.h>
// import O_RDONLY
#include <sys/stat.h>

#define BASIC_BUF 10
#define STANDARD_BUF 100
#define PREMIUM_BUF 1000
#define BUF_SIZE PREMIUM_BUF

#define MAX_CLNT 256

typedef struct {
    int command; // 0: FILE_REQ, 1: FILE_SENDING, 2: FILE_END, 3: FILE_END_ACK
    int type; // 0: BASIC, 1: STANDARD, 2: PREMIUM
    char buf[BUF_SIZE];
    int len;
} PACKET;

void* handle_clnt(void* arg);
void send_msg(char* msg, int len, int socketId);
void error_handling(char* msg);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT]; // 전역적으로 클라이언트 관리.
pthread_mutex_t mutx;

int main(int argc, char* argv[])
{
    // 기본 세팅 시작
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    pthread_t t_id;
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    printf("----------------------\n");
    printf(" K-OTT Service Server \n");
    printf("----------------------\n");

    pthread_mutex_init(&mutx, NULL);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while (1) {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

        // print ip address of clnt_socket
        printf("Connected client IP: %s , clnt_sock=%d\n", inet_ntoa(clnt_adr.sin_addr), clnt_sock);

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx);
        // 기본 세팅 끝

        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id); // 쓰레드 종료시 자동으로 소멸시킴. (자원해제)
    }

    pthread_mutex_destroy(&mutx);
    close(serv_sock);
    return 0;
}

int getBufSize(int type)
{
    if (type == 1)
        return BASIC_BUF;
    else if (type == 2)
        return STANDARD_BUF;
    else if (type == 3)
        return PREMIUM_BUF;
    else
        return BUF_SIZE;
}

// 서버는 현재 하나의 클라이언트 소켓에 대해 read와 write를 동시에 사용중이다.
void* handle_clnt(void* arg)
{
    int clnt_sock = *((int*)arg); // 현재 연결된 클라이언트
    int str_len = 0, file_read_size = 0, total_file_read_size = 0, i;
    char msg[BUF_SIZE]; // 원래 메시지
    PACKET packet;

    // 0. 제일 처음에 FILE_REQ 받아오기
    str_len = read(clnt_sock, &packet, sizeof(packet));
    // printf("packet.command: %d, packet.type: %d, str_len: %d\n", packet.command, packet.type, str_len); // 0, type, 1012 (1000 + 4 * 3)

    // ORIGIN: 현재 연결된 클라이언트로 부터 메시지를 받아서 모든 클라이언트에게 보내는 코드
    // while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
    //     send_msg(msg, str_len);

    // 1. 파일을 읽어서, 현재 연결된 클라이언트에게 보내기!
    // TODO: 아마 file 읽는것도 lock 을 걸어줘야 여러 클라이언트가 들어왔을때 문제가 없을듯.
    // TODO: 1. file read size 활용하는방법, 2. file 읽어서 보낼땐 type에 알맞게 보내야함. (file read size도 다르게해야함!? -> 보내는것만 다르게해야할걸?)
    packet.command = 1; // FILE_SENDING
    int fd = open("hw06.mp4", O_RDONLY);
    while ((packet.len = read(fd, packet.buf, getBufSize(packet.type))) != 0) {
        if (packet.len < getBufSize(packet.type))
            packet.command = 2; // FILE_END
        // send_msg(msg, file_read_size, clnt_sock);
        write(clnt_sock, &packet, sizeof(packet));
        total_file_read_size += packet.len;
        if (packet.command == 2)
            break;
    }

    // 2. 보낸 결과를 출력하기
    char* typeInString[4] = { "", "Basic", "Standard", "Premium" };
    printf("\nTotal Tx Bytes: %d to client %d (%s)\n", total_file_read_size, clnt_sock, typeInString[packet.type]);

    // 3. 마지막 FILE_END_ACK 받기
    read(clnt_sock, &packet, sizeof(packet));
    printf("[Rx] FILE_END_ACK from client %d => clnt_sock: %d closed.\n", clnt_sock, clnt_sock);
    if (packet.command == 3)
        return NULL;

    // 언제 작동되냐: 클라이언트가 종료되었을때 작동된다.
    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) // remove disconnected client
    {
        printf("clnt_sock: %d, clnt_cnt: %d\n", clnt_sock, clnt_cnt);
        if (clnt_sock == clnt_socks[i]) {
            while (i < clnt_cnt) {
                clnt_socks[i] = clnt_socks[i + 1];
                i++; // 클라이언트 종료시점에 무한루프 발생 문제점 수정
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);

    close(fd);
    close(clnt_sock);
    return NULL;
}

void send_msg(char* msg, int len, int socketId) // send to all -> send to specific socket -> no need
{
    int i;
    pthread_mutex_lock(&mutx);
    // ORIGIN: send to all code
    // for (i = 0; i < clnt_cnt; i++)
    //     write(clnt_socks[i], msg, len);

    // TODO: msg를 메시지 프로토콜에 삽입하여 알맞게 보낼 필요가 있음.
    write(socketId, msg, len);
    pthread_mutex_unlock(&mutx);
}
void error_handling(char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
