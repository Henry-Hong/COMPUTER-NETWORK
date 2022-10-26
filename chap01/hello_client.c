#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void error_handling(char *message);

int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in serv_addr;
  char message[2048] = {0};
  int str_len;

  if (argc != 3) {
    printf("Usage : %s <IP> <port>\n", argv[0]);
    exit(1);
  }

  sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock == -1) error_handling("socket() error");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_addr.sin_port = htons(atoi(argv[2]));

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    error_handling("connect() error!");

  // 통째로 read하기
  str_len = read(sock, message, sizeof(message) - 1);
  printf("str_len: %d, sizeof(message): %d\n", str_len, sizeof(message));
  if (str_len == -1) error_handling("read() error!");
  printf("Message from server: %s \n", message);

  // 1byte 씩 read하기!
  // int idx = 0;
  // long read_len = 0;
  // while (read_len = read(sock, &message[idx++], sizeof(char))) {
  //   printf("read_len: %d, %s\n", read_len, message);
  //   if (read_len == -1) {
  //     error_handling("read() error!");
  //     break;
  //   }
  //   str_len += read_len;
  // }
  // printf("Message from server! (read by 1 byte): %s\n", message);

  close(sock);
  return 0;
}

void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
