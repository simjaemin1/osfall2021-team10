#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdlib.h>
#include <linux/prinfo.h>

int main(int argc, char *argv[])
{
    int *nr = malloc(sizeof(int));
    *nr = 10;
    struct prinfo *prlist;
    prlist = (struct prinfo *)malloc(sizeof(struct prinfo) * (*nr));
	syscall(398, prlist, nr);

    // printf or something

    free(prlist);
    free(nr);
    return 0;
}
	
