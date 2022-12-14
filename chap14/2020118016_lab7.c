#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h> // htons(), htonl(), inet_addr(), inet_ntoa()
#include <signal.h> // signal()
#include <sys/socket.h> // socket(), bind(), listen(), accept(), connect()
#include <unistd.h> // close(), read(), write()

#define TEMP_SIZE 100
#define ON 1 // option on
#define OFF 0 // option off
#define TTL 64 // time to live
#define BUFFER_SIZE 150 // 100 + 50(이름)

void error_handling(char* message);

int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Usage : %s <GROUPIP> <PORT> <NAME>\n", argv[0]);
        exit(1);
    }

    int send_sock;
    int recv_sock;
    struct sockaddr_in recv_adr;
    struct sockaddr_in send_adr;
    struct ip_mreq join_adr;
    int str_len;
    int time_live = TTL;
    char buffer[BUFFER_SIZE];

    // sender socket setting
    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&send_adr, 0, sizeof(send_adr));
    send_adr.sin_family = AF_INET;
    send_adr.sin_addr.s_addr = inet_addr(argv[1]); // assign GroupIP
    send_adr.sin_port = htons(atoi(argv[2])); // assign PORT
    setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));

    // receiver socket setting
    recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&recv_adr, 0, sizeof(recv_adr));
    recv_adr.sin_family = AF_INET;
    recv_adr.sin_addr.s_addr = htonl(INADDR_ANY); // assign IP itself
    recv_adr.sin_port = htons(atoi(argv[2])); // assign PORT
    int reuse_opt = ON; // socket reuse option
    setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&reuse_opt, sizeof(reuse_opt));

    if (bind(recv_sock, (struct sockaddr*)&recv_adr, sizeof(recv_adr)) == -1)
        error_handling("bind() error");

    join_adr.imr_multiaddr.s_addr = inet_addr(argv[1]); // 멀티캐스트 그룹 주소
    join_adr.imr_interface.s_addr = htonl(INADDR_ANY); // 멀티캐스트 그룹에 가입할 자신의 IP 주소
    setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr)); // 멀티캐스트 그룹 가입

    pid_t pid = fork();

    // 프로세스 생성 에러
    if (pid == -1) {
        error_handling("fork() error");
    }
    // 자식 프로세스
    else if (pid == 0) {
        while (1) {
            str_len = recvfrom(recv_sock, buffer, BUFFER_SIZE - 1, 0, NULL, 0);
            if (str_len < 0)
                break;
            buffer[str_len] = '\0';
            printf("Received Message: %s", buffer);
        }
    }
    // 부모 프로세스
    else {
        while (1) {
            char temp[TEMP_SIZE];
            fgets(temp, TEMP_SIZE, stdin);
            if (!strcmp(temp, "Q\n") || !strcmp(temp, "q\n"))
                break;
            sprintf(buffer, "[%s]: %s", argv[3], temp);
            sendto(send_sock, buffer, strlen(buffer), 0, (struct sockaddr*)&send_adr, sizeof(send_adr));
        }
        setsockopt(recv_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));
        printf("Leave multicast group\n");
        kill(pid, SIGKILL);
    }

    close(recv_sock);
    close(send_sock);

    return 0;
}

void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}