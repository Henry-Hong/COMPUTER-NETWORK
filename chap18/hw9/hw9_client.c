#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 1000 // 사용자 유형에 따라 다르지만 MAX 로 설정
#define NAME_SIZE 20

void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);
void init();

typedef struct {
    int command; // 0: FILE_REQ, 1: FILE_SENDING, 2: FILE_END, 3: FILE_END_ACK
    int type; // 0: BASIC, 1: STANDARD, 2: PREMIUM
    char buf[BUF_SIZE];
    int len;
} PACKET;

int type = -1;
char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char* argv[])
{
    // 기본 세팅 시작
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void* thread_return;
    if (argc != 3) {
        // printf("Usage : %s <IP> <port> <name>\n", argv[0]);
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    init(); // connected가 언제 되는지 봐야함.
    if (type == 4) {
        printf("Exit program\n");
        exit(1);
    }
    // type이 1, 2, 3 이면 read 해서 socket 으로 command: 으로 해서 packet 보내기

    // sprintf(name, "[%s]", argv[3]);
    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    // 기본 세팅 끝
    // pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);

    // pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    close(sock);
    return 0;
}

// 사용안함
void* send_msg(void* arg) // send thread main
{
    int sock = *((int*)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];

    // 1. 제일 처음에 FILE_REQ 한번 보내주기
    PACKET packet;
    packet.command = 0;
    packet.type = type;
    packet.len = 0;
    write(sock, &packet, sizeof(packet));

    // 2. 계속 보내기. 근데 계속 보낼일이 있나? 없는데.. 마지막에 한번만 해주면 됨!
    while (1) {
        fgets(msg, BUF_SIZE, stdin);
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
            close(sock);
            exit(0);
        }
        sprintf(name_msg, "%s %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}

void* recv_msg(void* arg) // read thread main
{
    int read_size;
    unsigned long total_bytes = 0;
    int sock = *((int*)arg); // 위에서 연결된 소켓을 받아옴

    // 1. 제일 처음에 FILE_REQ 한번 보내주기
    PACKET packet;
    packet.command = 0;
    packet.type = type;
    packet.len = 0;
    write(sock, &packet, sizeof(packet));

    // 2. 시간 측정 시작
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    // 3. 파일 받기 until FILE_END
    while (1) {
        read_size = read(sock, &packet, sizeof(packet));
        total_bytes += packet.len;
        if (read_size == -1)
            return (void*)-1;
        fputs("*", stdout);
        if (packet.command == 2) // FILE_END
            break;
    }

    // 4. 시간 측정 종료
    clock_gettime(CLOCK_REALTIME, &end);
    double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    // 5. 끝나고 나서 FILE_END_ACK 보내주기
    packet.command = 3; // FILE_END_ACK
    write(sock, &packet, sizeof(packet));

    // 6. 나머지 출력
    printf("File Transmission Finished\n");
    printf("Total received bytes: %ld\n", total_bytes);
    printf("Downloading time: %.4lf sec\n", time);
    printf("Client closed\n");

    return NULL;
}

void error_handling(char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

void init()
{
    int menu;
    while (1) {
        printf("\n");
        printf("-----------------------------------------------\n");
        printf("                 K-OTT Service                 \n");
        printf("-----------------------------------------------\n");
        printf("Choose a subscribe type\n");
        printf("-----------------------------------------------\n");
        printf("1: Basic, 2: Standard, 3: Premium, 4: quit: ");
        scanf("%d", &type);
        if (type == 4)
            return;
        printf("1. Download, 2: Back to Main menu: ");
        scanf("%d", &menu);
        if (menu == 2)
            continue;
        return;
    }
}