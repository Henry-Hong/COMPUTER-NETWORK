#include <stdio.h>
#include <string.h>
#include <sys/uio.h>

int main()
{
    struct iovec vec[2];

    int CNT_BUFFER = 2;
    char str1[] = "My name is ";
    char str2[] = "Hong Hee Rim";
    int str_len;

    vec[0].iov_base = str1;
    vec[0].iov_len = strlen(str1);

    vec[1].iov_base = str2;
    vec[1].iov_len = strlen(str2);

    str_len = writev(1, vec, CNT_BUFFER);
    printf("\nwrite bytes: %d\n", str_len);

    return 0;
}