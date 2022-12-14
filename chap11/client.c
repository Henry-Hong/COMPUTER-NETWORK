#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SET 1
#define TRUE 1
#define FALSE 0
#define SENDER 1
#define RECEIVER 2
#define BUFFER_SIZE 2048
char* ROLE_BY_TYPE[] = { "", "Sender", "Receiver" };

void error_handling(char* message);

int main(int argc, char* argv[])
{
    int type;
    int fd1, fd2;
    char message[BUFFER_SIZE];
    int read_size, fd_max, fd_num;
    struct sockaddr_in serv_adr;

    fd_set reads, temp_reads;
    struct timeval timeout;

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    printf("----------------------------\n");
    printf("Choose function\n");
    printf("1. Sender,\t2: Receiver\n");
    printf("----------------------------\n");
    printf("=> ");

    scanf("%d", &type); // 1: sender, 2: receiver
    if (!(type == SENDER || type == RECEIVER))
        error_handling("Invalid Type! Choose 1 or 2");

    printf("\nFile %s Start!\n", ROLE_BY_TYPE[type]);

    FD_ZERO(&reads); // reads를 0으로 초기화

    if (type == SENDER) {
        fd1 = open("rfc1180.txt", O_RDONLY);
        if (fd1 == -1)
            error_handling("open() error");
        FD_SET(fd1, &reads); // fd1을 reads에 추가
    }

    fd2 = socket(PF_INET, SOCK_STREAM, 0);
    if (fd2 == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(fd2, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("connect() error!");
    else
        printf("Connected...........\n");
    write(fd2, ROLE_BY_TYPE[type], strlen(ROLE_BY_TYPE[type]));

    FD_SET(fd2, &reads);
    fd_max = fd2;

    if (type == SENDER)
        printf("fd1: %d, fd2: %d\n", fd1, fd2);
    else
        printf("fd2: %d\n", fd2);

    printf("fd_max: %d\n", fd_max);
    while (TRUE) {
        temp_reads = reads;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        if ((fd_num = select(fd_max + 1, &temp_reads, 0, 0, &timeout)) == -1)
            break;
        if (fd_num == 0)
            continue;

        if (type == SENDER) {
            if (FD_ISSET(fd2, &temp_reads)) {
                read_size = read(fd2, message, BUFFER_SIZE);
                write(1, message, read_size);
                if (read_size < BUFFER_SIZE) // ?
                    break;
            }
            if (FD_ISSET(fd1, &temp_reads)) {
                sleep(1);
                read_size = read(fd1, message, BUFFER_SIZE);
                if (read_size == 0) {
                    FD_CLR(fd1, &reads);
                    close(fd1);
                }
                write(fd2, message, read_size);
            }
        } else {
            if (FD_ISSET(fd2, &temp_reads)) {
                read_size = read(fd2, message, BUFFER_SIZE);
                write(1, message, read_size);
                write(fd2, message, read_size);
                if (read_size < BUFFER_SIZE) // ?
                    break;
            }
        }
    }
    FD_CLR(fd2, &reads); // fd2를 reads에서 제거
    close(fd2);
    return 0;
}

void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}