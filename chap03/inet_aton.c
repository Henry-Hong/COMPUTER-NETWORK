#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

// inet_aton : inet_address와 같음 but 같은 함수에서 실행됨.

int main()
{
    char* addr = "1.2.3.100";
    struct sockaddr_in addr_inet;

    if (!inet_aton(addr, &addr_inet.sin_addr))
        printf("Error....!!");
    else
        printf("Network ordered integer addr : %#x \n", addr_inet.sin_addr.s_addr);
    // 0x64030201

    return 0;
}