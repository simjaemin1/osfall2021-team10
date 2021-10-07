#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/prinfo.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	int *nr=(int*)malloc(sizeof(int));
	*nr = 200;
	struct prinfo *plist;
	struct prinfo p;
	plist = (struct prinfo*)malloc(sizeof(struct prinfo)*(*nr));
	int nr_k = syscall(398, plist, nr);
	for (int i=0; i<nr_k; i++)
	{
		p=plist[i];
		printf("%s,%d,%lld,%d,%d,%d,%lld\n", p.comm, p.pid, p.state, p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);
	}
}
	
