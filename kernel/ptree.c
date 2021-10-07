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
    int cnt; /* counted # of task */
    struct prinfo *buf_k; /* kernel prinfo buffer  */

    if(!buf || !nr) {
        printk("ERROR : Invalid argument (buf or nr is null)\n");
        return -EINVAL;
    }

    if(copy_from_user(&nr_k, nr, sizeof(int))) {
        printk("ERROR : nr reading error\n");
        return -EFAULT;
    }

    if(nr_k <= 1) {
        printk("ERROR : nr is less than 1\n");
        return -EINVAL;
    }

    buf_k = (struct prinfo *)kmalloc(nr_k * sizeof(struct prinfo), GFP_KERNEL);

    if(!buf_k) {
        printk("ERROR : struct prinfo buf error\n");
        return -EFAULT;
    }

    /* Lock tasklist and do something */
    read_lock(&tasklist_lock);

    /* End traversal. Unlock and copy to user space */
    read_unlock(&tasklist_lock);

    if(cnt < nr_k) nr_k = cnt;

    if(copy_to_user(nr, &nr_k, sizeof(int))) {
        printk("ERROR : copy to user space error (nr)\n");
        return -EFAULT;
    }

    if(copy_to_user(buf, &buf_k, sizeof(struct prinfo)*nr_k)) {
        printk("ERROR : copy to user space error (buf)\n");
        return -EFAULT;
    } 

    /* free kmalloc space and return */

    kfree(buf_k);

    return nr_k;
}

