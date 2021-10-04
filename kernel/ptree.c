#include <asm/uaccess.h>
#include <uapi/asm-generic/errno-base.h>
#include <linux/syscalls.h>
#include <linux/slab.h>     // kmalloc and kfree
#include <linux/list.h>
#include <linux/sched.h>    // task_struct (doubly linked list)
#include <linux/sched/task.h>
#include <linux/prinfo.h>

SYSCALL_DEFINE2(ptree, struct prinfo __user *, buf, int __user *, nr)
{
    printk("syscall ptree working test\n");
    return 0;
}

