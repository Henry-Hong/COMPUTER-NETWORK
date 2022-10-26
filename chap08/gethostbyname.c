#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void error_handling(char* message);
int main(int argc, char* argv[])
{
    int i;
    struct hostent* host;
    /*
      struct hostent
      {
        char *h_name;		// Official name
        char **h_aliases;	// Alias list -> 이 이름으로도 들어갈 수 있다.
        int h_addrtype;		// Host address type -> IPv4 / IPv6
        int h_length; // Address length -> 4(Ipv4) or 16(Ipv6)
        char **h_addr_list; // dotted-decimal 로 표현된 IP 주소 리스트
      }
    */

    if (argc != 2) {
        printf("Usage: %s <addr>\n", argv[0]);
        exit(1);
    }
    host = gethostbyname(argv[1]);
    if (!host)
        error_handling("gethost ... error");

    // 1. Official name
    printf("Official name: %s\n", host->h_name);

    // 2. Alias list
    for (i = 0; host->h_aliases[i]; i++)
        printf("Aliases %d: %s\n", i + 1, host->h_aliases[i]);

    // 3. Address type
    printf("Address type: %s\n", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");

    // 4. dotted-decimal 로 표현된 IP 주소 리스트
    for (i = 0; host->h_addr_list[i]; i++) {
        // printf("h_addr_list[%d]: 0x%x\n", i, (unsigned int)&host->h_addr_list[i]);
        printf("IP addr %d %s\n", i + 1, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
    }

    return 0;
}

void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
