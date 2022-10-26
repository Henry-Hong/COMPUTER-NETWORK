#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
typedef unsigned long u32;

// htonl: 32비트 정수형을 네트워크 바이트 순서로 변환
// htons: 16비트 정수형을 네트워크 바이트 순서로 변환

// inet_addr: 문자열로 된 IP 주소를 32비트 정수형으로 변환  + 네트워크 바이트로 변환 (big endian)
// inet_aton: 문자열로 된 IP 주소를 32비트 정수형으로 변환 + 네트워크 바이트로 변환 (big endian)
// inet_ntoa: 32비트 정수형을 문자열로된 IP 주소로 변환

int main()
{
    // char* original_str;
    // u32 conv_addr = 0x64030201;
    // struct sockaddr_in addr;

    // addr.sin_addr.s_addr = conv_addr;

    // original_str = inet_ntoa(addr.sin_addr);
    // printf("ascii address: %s\n", original_str);

    // 주소를 문자열로 선언
    char* addr1 = "1.2.3.100";
    char* addr2 = "1.2.3.101";

    // 문자열 to 32비트 정수형
    u32 addr1_int = inet_addr(addr1);
    u32 addr2_int;
    inet_aton(addr2, &addr2_int);

    printf("addr1_int: %#x\n", addr1_int);
    printf("addr2_int: %#x\n", addr2_int);

    // 32비트 정수형 to 네트워크 바이트
    struct sockaddr_in addr1_int_network;
    addr1_int_network.sin_addr.s_addr = (addr1_int);

    struct sockaddr_in addr2_int_network;
    addr2_int_network.sin_addr.s_addr = (addr2_int);
    // addr2_int_network.sin_addr.s_addr = htonl(addr2_int); // 이미 inet_addr 에서 다 했음.

    printf("addr1_int: %#x\n", addr1_int_network.sin_addr.s_addr);
    printf("addr2_int: %#x\n", addr2_int_network.sin_addr.s_addr);

    // 네트워크 바이트 정수 to 주소 문자열
    char* origin_addr1;
    char* origin_addr2;
    origin_addr1 = inet_ntoa(addr1_int_network.sin_addr);
    printf("ascii addr1: %s\n", origin_addr1); // origin_addr1의 값이 잘 나옴
    origin_addr2 = inet_ntoa(addr2_int_network.sin_addr);

    // printf("ascii add: %s\n", origin_addr1); // 여기서 출력해버리면, 같은 버퍼에 있는 값이 바뀌기 때문에 원하던 주소가 출력되지않음.
    printf("ascii addr2: %s\n", origin_addr2);

    return 0;
}