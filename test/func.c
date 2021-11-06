#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <time.h>
#include <sys/types.h>

#define SCHED_SETSCHEDULER 156
#define SCHED_SETWEIGHT    398
#define SCHED_GETWEIGHT    399
#define SCHED_WRR            7

struct sched_param {
    int sched_priority;
};

void print(int num) {
    if(num == 1) {
        printf("parent print called!!\n");
    }
    else {
        printf("child print called!!\n");
    }
}

int main() {
    int retval;
    struct sched_param param = {0};
    syscall(SCHED_SETSCHEDULER, getpid(), SCHED_WRR, &param);
    syscall(SCHED_SETWEIGHT, getpid(), 17);
    syscall(SCHED_GETWEIGHT, getpid());
    print(1);

    if(fork() == 0) {
        retval = syscall(SCHED_SETSCHEDULER, getpid(), SCHED_WRR, &param);
        syscall(SCHED_SETWEIGHT, getpid(), 14);
        syscall(SCHED_GETWEIGHT, getpid());
        print(0);
    }
}
