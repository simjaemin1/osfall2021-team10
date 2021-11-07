#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
//#include <uapi/linux/sched/types.h>

#define SCHED_SETSCHEDULER 156
#define SCHED_SETWEIGHT    398
#define SCHED_GETWEIGHT    399
#define SCHED_WRR            7

struct sched_param {
    int sched_priority;
};

void prime_factor(int num) {
    printf("Start prime_factor with num : %d\n", num);
    printf("%d = ", num);
    int i = 2;
    while(i < num) {
        if(num % i)
            i++;
        else {
            num /= i;
            printf("%d*", i);
        }
    }
    printf("%d\n", num);
}

int main(int argc, char *argv[]) {
    int num = 1029385961;
    int retval = 0;
    struct timespec begin, end;
    double time_interval;
    struct sched_param param = {0};

// 1, 5, 7, 11, 15, 19
    for(int i = 1; i < 11; i++) {
        int weight = i;
        if(fork() == 0) {
            int res = syscall(SCHED_SETSCHEDULER, getpid(), SCHED_WRR, &param);
            //printf("SCHED_SETSCHEDULER RESULT : %d\n", res);
            retval = syscall(SCHED_SETWEIGHT, getpid(), weight);
            if(retval < 0) {
                printf("SCHED_SETWEIGHT ERROR\n");
                exit(-1);
            }

            /*retval = syscall(SCHED_GETWEIGHT, getpid());
            if(retval < 0) {
                printf("SCHED_GETWEIGHT ERROR\n");
                exit(-1);
            }*/
            clock_gettime(CLOCK_MONOTONIC, &begin);
            prime_factor(num);
            clock_gettime(CLOCK_MONOTONIC, &end);
            time_interval = (double)(end.tv_sec - begin.tv_sec) +
                (double)(end.tv_nsec - begin.tv_nsec) / 1000000000;
            printf("Weight is %d and spend time %fsec\n", weight, time_interval);
            exit(0);         
        }
    }
    return 0;
}
