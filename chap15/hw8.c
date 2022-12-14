#include <fcntl.h> // open(), O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC, O_APPEND
#include <stdio.h>
#include <unistd.h> // dup(), close(), read(), write()
#define BUF_SIZE 1024
#define TRUE 1

// file descriptor 0 : stdin
// file descriptor 1 : stdout
// file descriptor 2 : stderr

int main(void)
{
    int fd1, fd2;
    FILE *fp2, *fp1;
    char buf1[BUF_SIZE], buf2[BUF_SIZE];

    // 첨부된 data1.txt 파일을 open()함수를 이용하여 열고, 해당 파일 디스크립터를 dup()함수를 이용하여 파일 디스크립터를 복제를 함.
    fd1 = open("data1.txt", O_RDONLY);
    fd2 = dup(fd1);

    // 2개의 파일 디스크립터를 fdopen()함수를 이용하여 FILE*로 변환함
    fp1 = fdopen(fd1, "r");
    fp2 = fdopen(fd2, "r");

    // 2개의 FILE*를 번갈아 사용하여 하나의 파일을 읽고 화면에 출력함: fgets(), fputs()
    while (TRUE) {
        if (fgets(buf1, BUF_SIZE, fp1) == NULL)
            break;
        fputs(buf1, stdout);
        fflush(fp1);
        if (fgets(buf2, BUF_SIZE, fp2) == NULL)
            break;
        fputs(buf2, stdout);
        fflush(fp2);
    }

    fclose(fp1);
    fclose(fp2);

    return 0;
}