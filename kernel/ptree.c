/* OSFALL2021 / TEAM10 / PROJ1 / Han Bin & Seung Hyun */

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
    /* Initiate variable, list with error check */

    int nr_k; /* kernel nr value */
    int ret; /* return value (counted # of task or nr) */
    struct prinfo buf_k; /* kernel prinfo buffer value */
    


    if(!buf || !nr) {
        printk("ERROR : Invalid argument (buf or nr is null)\n");
        return -EINVAL;
    }

    if(copy_from_user(&nr_k, nr, sizeof(int))) {
        printk("ERROR : nr reading error\n");
        return -EFAULT;
    }

    if(nr_k <= 0) {
        printk("ERROR : nr is not positive");
        return -EINVAL;
    }



    /* Lock tasklist and do something */
    read_lock(&tasklist_lock);

    read_unlock(&tasklist_lock);


    /* return  */
    ret = (ret > nr) ? nr : ret;
    return ret;
}

