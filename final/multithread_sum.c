#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define THREAD_NUM 10

void* thread_main(void* arg);

typedef struct {
    int start;
    int end;
} Ssang;

int sum = 0;

// mutex
pthread_mutex_t mutex;

int main(int argc, char* argv[])
{
    pthread_mutex_init(&mutex, NULL);

    pthread_t t_id[THREAD_NUM];

    Ssang ssangs[10] = { { 1, 10 }, { 11, 20 }, { 21, 30 }, { 31, 40 }, { 41, 50 }, { 51, 60 }, { 61, 70 }, { 71, 80 }, { 81, 90 }, { 91, 100 } };

    void* thr_ret;

    for (int i = 0; i < THREAD_NUM; i++)
        pthread_create(&t_id[i], NULL, thread_main, (void*)&(ssangs[i]));
    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_join(t_id[i], &thr_ret);
        // printf("Thread return message: %s\n", (char*)thr_ret);
    }

    printf("sum: %d\n", sum);

    free(thr_ret);
    pthread_mutex_destroy(&mutex);

    return 0;
}

void* thread_main(void* arg)
{
    struct timespec start_t, end_t;
    clock_gettime(CLOCK_REALTIME, &start_t);

    int start = (*(Ssang*)arg).start;
    int end = ((Ssang*)arg)->end;

    for (int i = start; i <= end; i++) {
        sum += i;
    }
    clock_gettime(CLOCK_REALTIME, &end_t);
    double time = (end_t.tv_sec - start_t.tv_sec) + (end_t.tv_nsec - start_t.tv_nsec) / 1000000000.0;
    printf("Downloading time: %.4lf sec\n", time);

    pthread_mutex_lock(&mutex);
    char* msg = calloc(10, sizeof(char));
    sprintf(msg, "[%d, %d] 더하기 끝남!", start, end);
    printf("%s\n", msg);
    pthread_mutex_unlock(&mutex);
    return (void*)msg;
}
