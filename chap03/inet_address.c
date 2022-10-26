#include <arpa/inet.h>
#include <stdio.h>
typedef unsigned long u32;

// inet_addr: 문자열로 된 IP 주소를 32비트 정수형으로 변환

int main()
{

    char* add1 = "1.2.3.100";
    char* add2 = "12.34.56.789";

    u32 conv_addr;

    conv_addr = inet_addr(add1);
    if (conv_addr == INADDR_NONE)
        printf("Error..!\n");
    else
        printf("Network ordered integer addr : %#lx \n", conv_addr); // 0x64030201

    conv_addr = inet_addr(add2);
    if (conv_addr == INADDR_NONE)
        printf("Error..!\n"); // ERROR
    else
        printf("Network ordered integer addr : %#lx \n", conv_addr);

    return 0;
}