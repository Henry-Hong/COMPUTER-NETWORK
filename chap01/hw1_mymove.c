#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define BYTES 4
#define MOD_NUMBER 0644

void handleError(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
        handleError("[Error] mymove Usage: ./mymove src_file dest_file");

    // 1. open src, dst file first
    int fdr = open(argv[1], O_RDONLY);
    if (fdr == -1)
        handleError("open() read	error!");

    int fdw = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, MOD_NUMBER);
    if (fdw == -1)
        handleError("open() write	error!");

    // 2. file read
    char buf[BYTES];
    int readSize = 0;
    int writeSize = 0;
    int totalWriteSize = 0;

    // 2-1. while break when readSize == 0
    // 장점: 버퍼 사이즈에 관계없이 입력받을 수 있음.
    while ((readSize = read(fdr, buf, sizeof(buf))) > 0) {
        if (readSize == -1)
            handleError("read() error!");

        writeSize = write(fdw, buf, sizeof(buf));
        if (readSize != writeSize)
            handleError("write()error!");

        totalWriteSize += writeSize; //총 이동된 파일의 크기 출력
    }

    // 2-2. read at once
    // readSize = read(fdr, buf, sizeof(buf));
    // if (readSize == -1)
    //     handleError("read() Error!");

    // writeSize = write(fdw, buf, readSize);
    // if (writeSize != readSize)
    //     handleError("write() Error!");

    // 4. remove read file
    // remove returns 0 if it succeed
    if (remove(argv[1]))
        handleError("file remove error!"); //  read()함수의 리턴값을 사용: 실제 읽은 바이트 수를 리턴함

    // 5. close file
    if (close(fdr) == -1)
        handleError("fail to close read file");
    if (close(fdw) == -1)
        handleError("fail to close write file");

    printf("move from %s to %s (bytes: %d) finished.", argv[1], argv[2],
        totalWriteSize);

    return 0;
}