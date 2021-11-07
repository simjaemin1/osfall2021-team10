/* OSFALL2021 TEAM 10 PROJ2 / KOO HAN BIN & LEE SEUNG HYOUN */

/* kernel/sched/wrr.c */

#include "sched.h"

#include <linux/slab.h>     // kmalloc
#include <linux/kernel.h>   // printk for debugging
#include <linux/list.h>     // for list operation (queue)
#include <linux/jiffies.h>  // get_jiffies_64()
#include <linux/sched/wrr.h>

void init_wrr_rq(struct wrr_rq *wrr_rq)
{
    INIT_LIST_HEAD(&wrr_rq->queue_head);
    wrr_rq->total_weight = 0;
    wrr_rq->load_balanced_time = get_jiffies_64();
}

void trigger_load_balance_wrr(struct rq *rq)
{
  
    /* values need for jiffies  */
    struct wrr_rq *wrr_rq = &rq->wrr;
    unsigned long prev_time = wrr_rq->load_balanced_time;
    unsigned long curr_time = get_jiffies_64();

    /* values need for max & min wrr_rq  */
    int cpu;
    int max_cpu = 0;
    int min_cpu = 0;
    unsigned int min_total_weight = 0xFFFFFFFF;
    unsigned int max_total_weight = 0;
    struct wrr_rq *curr_rq;
    struct wrr_rq *max_wrr_rq = NULL;
    struct wrr_rq *min_wrr_rq = NULL;

    /* values need for traverse & check migration  */
    unsigned long flags;
    struct list_head *head;
    struct sched_wrr_entity *wrr_se;
    struct sched_wrr_entity *wrr_se_to_migrate = NULL;
    struct task_struct *p;
    unsigned int weight;
    unsigned int max_weight = 0;

    /* Check 2000ms to do load balancing  */

    if(time_before(curr_time, prev_time + WRR_PERIOD)) {
        /* 2000ms did not passed  */
        return;
    }
    wrr_rq->load_balanced_time = curr_time;

    /* 2000ms passed. Do Load Balancing  */
    /* First, find max & min total_weight wrr_rq  */

    rcu_read_lock();
    for_each_online_cpu(cpu) {
        if(cpu == CPU_WITHOUT_WRR)
            continue;
        curr_rq = &(cpu_rq(cpu)->wrr);
        if(curr_rq->total_weight > max_total_weight) {
            max_cpu = cpu;
            max_total_weight = curr_rq->total_weight;
        }
        if(curr_rq->total_weight < min_total_weight) {
            min_cpu = cpu;
            min_total_weight = curr_rq->total_weight;
        }
    }
    max_wrr_rq = &(cpu_rq(max_cpu)->wrr);
    min_wrr_rq = &(cpu_rq(min_cpu)->wrr);
    rcu_read_unlock();
    
    if(max_cpu == min_cpu || !max_wrr_rq || !min_wrr_rq) {
        /* Casie 1   : Don't need to migrate  */
        /* Case 2,3 : No CPUs available  */
        return;
    }

    /* traverse and migrate if possible */
    local_irq_save(flags);
    double_rq_lock(cpu_rq(max_cpu), cpu_rq(min_cpu));
    
    head = &max_wrr_rq->queue_head;
    
    list_for_each_entry(wrr_se, head, node) {
        weight = wrr_se->weight;
        p = container_of(wrr_se, struct task_struct, wrr);
        /* Tasks currently running are not eligible for migration  */
        if(cpu_rq(max_cpu)->curr == p)
            continue;
        /* Check whether cpu is available  */
        if(!cpumask_test_cpu(max_cpu, &p->cpus_allowed))
            continue;
        /* Check total weight condition after migrate  */
        if(max_total_weight - weight <= min_total_weight + weight)
            continue;
        /* We have to choose maximum migration weight  */
        if(weight <= max_weight)
            continue;
        /* Satisfied all the condition.  */
        max_weight = weight;
        wrr_se_to_migrate = wrr_se;
    }

    if(wrr_se_to_migrate) {
        /* Successfully chose task to migrate  */
        p = container_of(wrr_se_to_migrate, struct task_struct, wrr);
        deactivate_task(cpu_rq(max_cpu), p, 0);
        set_task_cpu(p, min_cpu);
        activate_task(cpu_rq(min_cpu), p, 0);
        resched_curr(cpu_rq(min_cpu));
    }
    else {
        /* No proper task to migrate. Do nothing */
    }

    double_rq_unlock(cpu_rq(max_cpu), cpu_rq(min_cpu));
    local_irq_restore(flags);
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{  
    struct wrr_rq *wrr_rq = &rq->wrr;
    struct sched_wrr_entity *wrr_se = &p->wrr;

    INIT_LIST_HEAD(&wrr_se->node);
    list_add_tail(&wrr_se->node, &wrr_rq->queue_head);

    wrr_se->on_rq = 1;
    wrr_se->timeslice = wrr_se->weight * WRR_TIMESLICE;

    /* For the load balancing  */
    wrr_rq->total_weight += wrr_se->weight;
    add_nr_running(rq, 1);

    resched_curr(rq);
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    struct wrr_rq *wrr_rq = &rq->wrr;
    struct sched_wrr_entity *wrr_se = &p->wrr;

    list_del_init(&wrr_se->node);
    wrr_se->on_rq = 0;

    wrr_rq->total_weight -= wrr_se->weight;

    sub_nr_running(rq, 1);
    resched_curr(rq);    
}

static struct task_struct *pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
    struct wrr_rq *wrr_rq;
    wrr_rq  = &rq->wrr;
    if(list_empty(&wrr_rq->queue_head)) {
        /* If wrr runqueue is empty, not able to pick next task. Just return NULL.  */
        return NULL;
    }

    struct sched_wrr_entity *picked = list_first_entry(&wrr_rq->queue_head, struct sched_wrr_entity, node);
    struct task_struct *p = container_of(picked, struct task_struct, wrr);
    
    return p;
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *prev)
{
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
    struct wrr_rq *wrr_rq;
    struct sched_wrr_entity *wrr_se;
    if(p->policy != SCHED_WRR)
        return;

    wrr_rq = &rq->wrr;
    wrr_se = &p->wrr;
    if(--wrr_se->timeslice) {
        /* Timeslice of this task remain.  */
        return;
    }
    else {
        /* Timeslice expired -> reset timeslice and requeue this task */
		list_del(&wrr_se->node);
        wrr_se->timeslice = wrr_se->weight * WRR_TIMESLICE;
        list_add_tail(&wrr_se->node, &wrr_rq->queue_head);

        resched_curr(rq);
    }
}

static void task_fork_wrr(struct task_struct *p)
{
    if(p == NULL) return;
    p->wrr.weight = p->real_parent->wrr.weight;
    p->wrr.timeslice = p->wrr.weight * WRR_TIMESLICE;
}
static void set_curr_task_wrr(struct rq *rq)
{
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
}

static int select_task_rq_wrr(struct task_struct *p, int cpu, int sd_flag, int flags)
{
    int iter_cpu;
    int min_cpu;
    int min_total_weight = 0xFFFFFFFF;
    struct wrr_rq *curr_rq;

    if(cpu != CPU_WITHOUT_WRR)
        return cpu;
    min_cpu = 0; // Just initialization
    rcu_read_lock();
    for_each_online_cpu(iter_cpu) {
        if(cpu == CPU_WITHOUT_WRR)
            continue;
        curr_rq = &(cpu_rq(iter_cpu)->wrr);
        if(curr_rq->total_weight < min_total_weight) {
            min_cpu = iter_cpu;
            min_total_weight = curr_rq->total_weight;
        }
    }
    rcu_read_unlock();
    return min_cpu;
}
static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags)
{
}

static void yield_task_wrr(struct rq *rq)
{
}

static void update_curr_wrr(struct rq *rq)
{
}

static void migrate_task_rq_wrr(struct task_struct *p)
{
}

static void rq_online_wrr(struct rq *rq)
{
}

static void rq_offline_wrr(struct rq *rq)
{
}
static void task_woken_wrr(struct rq *rq, struct task_struct *p)
{
}

static void task_dead_wrr(struct task_struct *p) 
{
}
const struct sched_class wrr_sched_class = {
    .next = &fair_sched_class,
    .enqueue_task = enqueue_task_wrr,
    .dequeue_task = dequeue_task_wrr,
    .yield_task = yield_task_wrr,
    .pick_next_task = pick_next_task_wrr,
    .put_prev_task = put_prev_task_wrr,
    .update_curr = update_curr_wrr,
    
    .select_task_rq = select_task_rq_wrr,
    .migrate_task_rq = migrate_task_rq_wrr,
    .rq_online = rq_online_wrr,
    .rq_offline = rq_offline_wrr,
    .task_woken = task_woken_wrr,
    .set_cpus_allowed = set_cpus_allowed_common,

    .task_tick = task_tick_wrr,
    .task_fork = task_fork_wrr,
	.set_curr_task = set_curr_task_wrr,	//used in setschedule
	.switched_to = switched_to_wrr,		//used in setschedule
	.check_preempt_curr = check_preempt_curr_wrr,
    .task_dead = task_dead_wrr, 
};
