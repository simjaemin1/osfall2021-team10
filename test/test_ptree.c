#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/prinfo.h>
#include <stdio.h>


void print_process_tree(struct prinfo *plist, int size)
{
	struct prinfo p;
	int *pid_stack=(int*) malloc(sizeof(int)*size);
	int ind;
	p = plist[0];
	printf("%s,%d,%lld,%d,%d,%d,%lld\n", p.comm, p.pid, p.state, p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);
	pid_stack[0] =p.pid;
	for (int i=1; i<size; i++)
	{
		ind=0;
		p=plist[i];
		do{printf("   ");}while(pid_stack[ind++]!=p.parent_pid);
		printf("%s, %d, %lld, %d, %d, %d, %lld\n", p.comm, p.pid, p.state, p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);
		pid_stack[ind]=p.pid;
	}

}


int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("Usage: /test (number of entries)");
		return -1;
	}
	int *nr=(int*)malloc(sizeof(int));
	*nr = atoi(argv[1]);
	struct prinfo *plist = (struct prinfo*)malloc(sizeof(struct prinfo)*(*nr));
	
	int total = syscall(398, plist, nr);
	print_process_tree(plist, *nr);
    printf("*************************\n");
    printf("nr is %d\n", *nr);
    printf("total # of process is %d\n", total);
    printf("*************************\n");
	free(nr);
	free(plist);
}


