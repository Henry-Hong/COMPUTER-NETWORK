// 쓰레드는 프로세스가 종료되면 강제로 같이 종료됨
// 따라서 쓰레드가 종료되기 전에 프로세스가 종료되지 sleep(2) -> sleep(10)으로 바꿈
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void* thread_main(void* arg);

int main(int argc, char* argv[])
{
    pthread_t t_id;
    int thread_param = 5;

    if (pthread_create(&t_id, NULL, thread_main, (void*)&thread_param) != 0) {
        puts("pthread_create() error");
        return -1;
    }
    sleep(10);
    // sleep(2);
    puts("end of main");
    return 0;
}

void* thread_main(void* arg)
{
    int i;
    int cnt = *((int*)arg);

    for (i = 0; i < cnt; i++) {
        sleep(1);
        puts("running thread");
    }

    return NULL;
}
