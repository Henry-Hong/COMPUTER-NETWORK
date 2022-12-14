#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BASIC_BUF 10
#define STANDARD_BUF 20
#define PREMIUM_BUF 30
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

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx);
        // 기본 세팅 끝

        // TODO: 쓰레드 대신 프로세스로 구현해보기
        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id); // 쓰레드 종료시 자동으로 소멸시킴. (자원해제)
        // pthread_join(t_id, NULL);

        // print ip address of clnt_socket
        printf("Connected client IP: %s , clnt_sock=%d\n", inet_ntoa(clnt_adr.sin_addr), clnt_sock);
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
    int fd = -1;
    PACKET packet;

    return NULL;

    // 0. 제일 처음에 FILE_REQ 받아오기
    fd = open("hw06.mp4", O_RDONLY);
    while (1) {
        read(clnt_sock, &packet, sizeof(packet));
        printf("Client pick: %s\n", packet.buf);

        char msg[100];
        if (fd == -1) {
            printf("1");
            packet.command = 1; // fail
            sprintf(msg, "\'%s\' you requested is not found\n", packet.buf);
            strcpy(packet.buf, msg);
            write(clnt_sock, &packet, sizeof(packet));
            continue;
        } else {
            printf("2");
            packet.command = 2; // success
            sprintf(msg, "\'%s\' you requested is found, file sending!\n", packet.buf);
            strcpy(packet.buf, msg);
            write(clnt_sock, &packet, sizeof(packet));
            break;
        }
    }

    // file sending
    int cnt = 0;
    packet.command = 3; // file sending
    int total_bytes = 0;
    while (1) {
        packet.len = read(fd, packet.buf, getBufSize(packet.type));
        if (packet.len == -1) {
            packet.command = 4; // file end
            write(clnt_sock, &packet, sizeof(packet));
            break;
        }
        cnt++;
        total_bytes += packet.len;
        write(clnt_sock, &packet, sizeof(packet));
    }

    // file end
    printf("Total bytes %d to client(%d), cnt = %d\n", total_bytes, clnt_sock, cnt);

    // file end ack
    read(clnt_sock, &packet, sizeof(packet));
    printf("[Rx] FILE_END_ACK from client %d => clnt_sock: %d closed.\n", clnt_sock, clnt_sock);

    // 클라이언트가 종료되고 나서 실행해준다!
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; i++) // remove disconnected client
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
