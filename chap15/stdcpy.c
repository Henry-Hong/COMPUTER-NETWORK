#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#define BUF_SIZE 3

int main(int argc, char* argv[])
{
    FILE* f1;
    FILE* f2;
    char buf[BUF_SIZE];
    double t1, t2;
    int cnt = 0;

    f1 = fopen("news.txt", "r");
    f2 = fopen("cpy.txt", "w");

    while (fgets(buf, BUF_SIZE, f1) != NULL)
        fputs(buf, f2);

    fclose(f1);
    fclose(f2);
    return 0;
}
