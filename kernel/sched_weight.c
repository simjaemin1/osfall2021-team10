#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/thread_info.h>


SYSCALL_DEFINE0(sched_setweight)
{
    return 0;
}

SYSCALL_DEFINE0(sched_getweight)
{
    p = get_current();
    printk("**** task name is %s ****\n",p->comm);
    printk("**** task pid  is %d ****\n",p->pid);
    printk("**** task wrr weight is %u ****\n", p->wrr.weight);
    return p->wrr.weight;
}
