#include <arpa/inet.h>
#include <stdio.h>

typedef unsigned short u16;
typedef unsigned long u32;

int main()
{

    u16 host_port = 0x1234;
    u16 network_port;

    u32 host_addr = 0x12345678;
    u32 network_addr;

    network_port = htons(host_port);
    network_addr = htonl(host_addr);

    printf("Host Port: %#X\n", host_port);
    printf("Netw Port: %#X\n", network_port);
    printf("Host Addr: %#lX\n", host_addr);
    printf("Netw Addr: %#lX\n", network_addr);

    return 0;
}