#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#define SCHED_SETSCHEDULER 156
#define SCHED_SETWEIGHT    398
#define SCHED_GETWEIGHT    399
#define SCHED_WRR            7

struct sched_param {
    int sched_priority;
};

int main(int argc, char *argv[]) {
    int retval;
    int child_status;
    struct sched_param param = {0};

    if(fork() == 0) {
        retval = syscall(SCHED_SETSCHEDULER, getpid(), SCHED_WRR, &param);
        printf("SCHED_SETSCHEDULER RESULT : %d\n", retval);
        retval = syscall(SCHED_SETWEIGHT, getpid(), 17);
        if(retval < 0) {
            printf("SCHED_SETWEIGHT ERROR\n");
            exit(-1);
        }

        retval = syscall(SCHED_GETWEIGHT, getpid());
        if(retval < 0) {
            printf("SCHED_GETWEIGHT ERROR\n");
            exit(-1);
        }
        exit(0);
    }
    pid_t wpid = waitpid(-1, &child_status, 0);
    if(WIFEXITED(child_status))
        printf("Child process successfully completed\n");
    else
        printf("Child process error\n");
    return 0;
}
