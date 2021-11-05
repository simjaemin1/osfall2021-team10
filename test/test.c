#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <linux/syscalls.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    pid_t pid = getpid();
    int weight1 = syscall(399, pid);
    printf("Before setweight : %d\n", weight1);
    int weight2 = 18;
    int weight3 = syscall(398, pid, weight2);
    printf("After setweight : %d\n", weight3);

    return 0;
}
