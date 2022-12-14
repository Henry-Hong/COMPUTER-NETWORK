#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define SET 1
#define TRUE 1
#define FALSE 0
#define SENDER 1
#define RECEIVER 2
#define BUFFER_SIZE 2048
char* ROLE_BY_TYPE[] = { "", "Sender", "Receiver" };
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void error_handling(char* message);

int main(int argc, char* argv[])
{
    fd_set reads, temp_reads;
    struct timeval timeout;

    struct sockaddr_in serv_adr, clnt_adr;
    int serv_sock, clnt_sock, sender_sock, receiver_sock;

    socklen_t adr_sz;
    int read_size, fd_max, fd_num;
    char buffer[BUFFER_SIZE];
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    FD_ZERO(&reads); // reads를 0으로 초기화
    FD_SET(serv_sock, &reads); // serv_sock을 reads에 추가
    fd_max = serv_sock; // fd_max를 serv_sock으로 초기화

    while (TRUE) {
        temp_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5 * 1000 * 100; // microseconds -> 500ms = 0.5s
        if ((fd_num = select(fd_max + 1, &temp_reads, 0, 0, &timeout)) == -1)
            break; // error
        if (fd_num == 0)
            continue; // timeout

        // serv_sock에 이벤트가 발생했을 때
        if (FD_ISSET(serv_sock, &temp_reads)) {
            adr_sz = sizeof(clnt_adr);
            clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
            FD_SET(clnt_sock, &reads);
            fd_max = MAX(fd_max, clnt_sock);

            // 클라이언트가 sender인지 receiver인지 확인
            printf("connected client: %d\n", clnt_sock);
            read_size = read(clnt_sock, buffer, BUFFER_SIZE);
            buffer[read_size] = '\0';
            if (strcmp(buffer, ROLE_BY_TYPE[SENDER]) == 0) {
                sender_sock = clnt_sock;
            } else if (strcmp(buffer, ROLE_BY_TYPE[RECEIVER]) == 0) {
                receiver_sock = clnt_sock;
            } else {
                printf("Invalid Client Type! Choose 1 or 2: %s\n", buffer);
                FD_CLR(clnt_sock, &reads); // reads에서 제거
                close(clnt_sock);
            }
        }
        // sender_sock에 이벤트가 발생했을 때
        else if (FD_ISSET(sender_sock, &temp_reads)) {
            read_size = read(sender_sock, buffer, BUFFER_SIZE);
            if (read_size == 0) {
                printf("closed client: %d\n", sender_sock);
                FD_CLR(sender_sock, &reads);
                close(sender_sock);
            } else {
                printf("Forward  [%d] ---> [%d]\n", sender_sock, receiver_sock);
                write(receiver_sock, buffer, read_size);
            }
        }
        // receiver_sock에 이벤트가 발생했을 때
        else if (FD_ISSET(receiver_sock, &temp_reads)) {
            read_size = read(receiver_sock, buffer, BUFFER_SIZE);
            if (read_size == 0) {
                printf("closed client: %d\n", receiver_sock);
                FD_CLR(receiver_sock, &reads);
                close(receiver_sock);
            } else {
                printf("Backward [%d] <--- [%d]\n", receiver_sock, sender_sock);
                write(sender_sock, buffer, read_size);
            }
        }
    }

    close(serv_sock);
    return 0;
}

void error_handling(char* buffer)
{
    fputs(buffer, stderr);
    fputc('\n', stderr);
    exit(1);
}