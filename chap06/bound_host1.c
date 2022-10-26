#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 30
void error_handling(char* message);

int main(int argc, char* argv[])
{
    int sock;
    char message[BUF_SIZE];
    struct sockaddr_in my_adr, your_adr;
    socklen_t adr_sz;
    int str_len, i;
    int total_len = 0;
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&my_adr, 0, sizeof(my_adr));
    my_adr.sin_family = AF_INET;
    my_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_adr.sin_port = htons(atoi(argv[1]));

    if (bind(sock, (struct sockaddr*)&my_adr, sizeof(my_adr)) == -1)
        error_handling("bind() error");

    // 데이터의 경계가 있는 UDP는 recvfrom 3회 각각이 sendto 3회와 매핑된다.
    // 참고로 host1이 server 같은느낌이고 host2가 client 같은느낌이다. (UDP는 알다시피 연결과정이없다.)
    for (i = 0; i < 3; i++) {
        sleep(1); // delay 5 sec.
        adr_sz = sizeof(your_adr);
        str_len = recvfrom(sock, message, BUF_SIZE, 0,
            (struct sockaddr*)&your_adr, &adr_sz);
        total_len += str_len;
        printf("Message %d: %s \n", i + 1, message);
    }
    printf("Total received bytes: %d\n", total_len);
    close(sock);
    return 0;
}

void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

/*
root@my_linux:/home/swyoon/tcpip# gcc bound_host1.c -o host1
root@my_linux:/home/swyoon/tcpip# ./host1
Usage : ./host1 <port>
root@my_linux:/home/swyoon/tcpip# ./host1 9190
Message 1: Hi!
Message 2: I'm another UDP host!
Message 3: Nice to meet you
root@my_linux:/home/swyoon/tcpip#

*/