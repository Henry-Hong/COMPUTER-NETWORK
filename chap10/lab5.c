// 2020118016 홍희림
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#define PARENT_TIME_INTERVAL 2
#define CHILD_TIME_INTERVAL 5

// SIGALRM : alarm signal
// SIGCHLD : child process exit signal
// SIGINT : interrupt signal (ctrl + c)
// SIGKILL : kill signal
// SIGSTOP : stop signal
// SIGTSTP : stop signal (ctrl + z)
// SIGCONT : continue signal

void childTimer(int sig)
{
    static int elapsed = 0;
    elapsed += CHILD_TIME_INTERVAL;
    if (sig == SIGALRM)
        printf("[Child]\t\ttime out: %d, elapsed time:\t%2d seconds\n", CHILD_TIME_INTERVAL, elapsed);
    alarm(5);
}
void parentTimer(int sig)
{
    static int elapsed = 0;
    elapsed += PARENT_TIME_INTERVAL;
    if (sig == SIGALRM)
        printf("<Parent>\ttime out: %d, elapsed time:\t%2d seconds\n", PARENT_TIME_INTERVAL, elapsed);
    alarm(2);
}
void childExitHandler(int sig)
{
    int status; // child process exit status
    pid_t id = waitpid(-1, &status, WNOHANG); // wait for child process exit
    if (WIFEXITED(status)) // if child process exit normally
        printf("Child id: %d, sent: %d\n", id, WEXITSTATUS(status));
}

int main()
{
    struct sigaction childAct; // child process
    childAct.sa_handler = childExitHandler; // child process exit
    sigemptyset(&childAct.sa_mask); // empty mask
    childAct.sa_flags = 0; // no flag
    sigaction(SIGCHLD, &childAct, 0); // register child process exit handler

    pid_t pid = fork(); // fork child process
    struct sigaction act; // action for child & parent process
    // handler will be added
    sigemptyset(&act.sa_mask); // empty mask
    act.sa_flags = 0; // no flag

    switch (pid) {
    case -1:
        puts("fork error");
        exit(1);
    case 0: // child process
        act.sa_handler = childTimer;
        sigaction(SIGALRM, &act, 0);
        alarm(CHILD_TIME_INTERVAL);
        for (int i = 0; i < 20; i++)
            sleep(1);
        return 5;
    default: // parent process
        act.sa_handler = parentTimer;
        sigaction(SIGALRM, &act, 0);
        alarm(PARENT_TIME_INTERVAL);
        while (1) // infinite loop
            sleep(1);
    }
    return 0;
}