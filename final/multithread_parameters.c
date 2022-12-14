#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char name[20];
    int age;
    double height;
    double weight;
    char address[100];
    int type; // 1: student, 2: teacher
} User;

typedef struct {
    int korean;
    int math;
} Score;

void* thread_main(void* arg);

int main(int argc, char* argv[])
{
    pthread_t t_id;
    void* thr_ret;

    // type1: 파라미터를 정수형변수로
    int thread_param = 5;

    // type2: 파라미터를 정수형배열로
    int thread_params[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // type3: 파라미터를 구조체로
    User user = { "Kim", 20, 180.5, 70.5, "Seoul", 1 };

    // type4: 파라미터를 구조체배열로
    User users[5]
        = {
              { "Kim", 20, 180.5, 70.5, "Seoul", 1 },
              { "Lee", 30, 170.5, 60.5, "Busan", 1 },
              { "Park", 40, 160.5, 50.5, "Incheon", 1 },
              { "Choi", 50, 150.5, 40.5, "Gwangju", 1 },
              { "Jung", 60, 140.5, 30.5, "Daejeon", 1 }
          };

    pthread_create(&t_id, NULL, thread_main, (void*)users);

    pthread_join(t_id, &thr_ret);

    printf("ages:\n");
    for (int i = 0; i < 5; i++) {
        printf("%d: %d\n", i, ((int*)thr_ret)[i]);
    }

    free(thr_ret);
    return 0;
}

void* thread_main(void* arg)
{
    // 구조체 정보를 출력하는 코드
    User* users = (User*)arg;
    int* ages = calloc(5, sizeof(int));

    for (int i = 0; i < 5; i++) {
        printf("Name: %s\n", users[i].name);
        ages[i] = users[i].age;
    }

    return (void*)ages;
}