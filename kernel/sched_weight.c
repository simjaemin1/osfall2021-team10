#include <asm/unistd.h>
#include <linux/types.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/thread_info.h>
#include <uapi/asm-generic/errno-base.h>
#include <asm/unistd.h> // uid & euid

#define WRR_MAX_WEIGHT 20
#define WRR_MIN_WEIGHT  1

/******************************************************
 * Things we should know!!
 *
 * Not yet checked validarity of this function.
 * We should move this function to core.c
 *
 *****************************************************/

SYSCALL_DEFINE2(sched_setweight, pid_t, pid, int, weight)
{
    struct task_struct *p;

    if(pid < 0 || weight < WRR_MIN_WEIGHT || weight > WRR_MAX_WEIGHT) {
        printk(KERN_ERR "ERROR : Invalid argument\n");
        return -EINVAL;
    }

    rcu_read_lock();

    p = find_task_by_vpid(pid);
    if(getuid()) {
        /* Permission Denied */
        printk(KERN_ERR "Permission Denied : You are not admin\n");
        rcu_read_unlock();
        return -EACCES;
    }
    else {
        /* Permission Accepted  */
        if(p->policy != SCHED_WRR) {
            printk(KERN_ERR "ERROR : Policy is not sched_wrr\n");
            rcu_read_unlock();
            return -EINVAL;
        }
        printk("Permission Accepted\n");
        p->wrr.weight = weight;
    }

    rcu_read_unlock();

    return weight;
}

SYSCALL_DEFINE1(sched_getweight, pid_t, pid)
{
    int weight;

    if(pid < 0) {
        printk(KERN_ERR "ERROR : Invalid pid\n");
        return -EINVAL;
    }

    rcu_read_lock();

    struct task_struct *p = find_task_by_vpid(pid);
    printk("**** task name is %s ****\n",p->comm);
    printk("**** task pid  is %d ****\n",p->pid);
    printk("**** task wrr weight is %u ****\n", p->wrr.weight);
    weight = p->wrr.weight;

    rcu_read_unlock();

    return weight;
}
