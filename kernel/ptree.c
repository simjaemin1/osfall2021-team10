/* OSFALL2021 / TEAM10 / PROJ1 / Koo Han Bin & Lee Seung Hyun */

#include <asm/uaccess.h>
#include <uapi/asm-generic/errno-base.h>
#include <linux/syscalls.h>
#include <linux/slab.h>     // kmalloc and kfree
#include <linux/list.h>
#include <linux/sched.h>    // task_struct (doubly linked list)
#include <linux/sched/task.h>
#include <linux/prinfo.h>
#include <linux/thread_info.h>	// get_current()

void store_prinfo(struct task_struct *task, struct prinfo *info) {
    info->state = task->state;
    info->pid = task->pid;
    info->parent_pid = task->parent->pid;
    info->first_child_pid = list_empty(&task->children) ? 0:list_first_entry(&task->children, struct task_struct, sibling)->pid;
    info->next_sibling_pid = list_empty(&task->sibling) ? 0:list_next_entry(task, sibling)->pid;
    info->uid = (int64_t)(task->cred->uid.val);
    strcpy(info->comm, task->comm);
}


void print_prinfo(struct prinfo *info) {
    /* Only for debugging purpose */
	printk("********* print prinfo called **********\n");
    printk("state 				: %lld\n", info->state);
	printk("pid 				: %d\n", info->pid);
    printk("parent pid 			: %d\n", info->parent_pid);
    printk("first child pid 	: %d\n", info->first_child_pid);
    printk("next sibling pid 	: %d\n", info->next_sibling_pid);
    printk("uid 				: %lld\n", info->uid);
    printk("comm 				: %s", info->comm);
    printk("****************************************\n");
}


/* traverse function */
int traverse(struct prinfo* buf_k, int nr_k)
{
	int cnt = 0;
	struct task_struct *task = &init_task;
  
	while(1) {
  	    if(cnt < nr_k) { 
		    store_prinfo(task, &buf_k[cnt++]);
	    }
        else { cnt++; }
    
  	    if(list_empty(&task->children)) {
        	while(list_is_last(&task->sibling,&task->parent->children)) { task = task->parent; }
		    task = list_next_entry(task, sibling);
        }
        else {
    	    task = list_first_entry(&task->children, struct task_struct, sibling);
        }
        
        if(task->pid == init_task.pid) break; /* if dfs reach at the top again */
    }

    return cnt;
}


SYSCALL_DEFINE2(ptree, struct prinfo __user *, buf, int __user *, nr)
{
    /* Initiate variable, list with error check */
    int nr_k; 			    /* kernel nr value 	    */
    int cnt = 0; 			/* counted # of task 	*/
    struct prinfo *buf_k; 	/* kernel prinfo buffer */

    
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
    
    cnt = traverse(buf_k, nr_k);
    
    /* End traversal. Unlock and copy to user space */
    read_unlock(&tasklist_lock);

    if(cnt < nr_k) nr_k = cnt;
	
    if(copy_to_user(nr, &nr_k, sizeof(int))) {
        printk("ERROR : copy to user space error (nr)\n");
        return -EFAULT;
    }
    if(copy_to_user(buf, buf_k, sizeof(struct prinfo)*nr_k)) {
        printk("ERROR : copy to user space error (buf)\n");
        return -EFAULT;
    } 
		
    /* free kmalloc space and return */

    kfree(buf_k);

    return cnt;
}

