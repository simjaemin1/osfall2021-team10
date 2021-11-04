/* OSFALL2021 TEAM 10 PROJ2 / KOO HAN BIN & LEE SEUNG HYOUN */

/* kernel/sched/wrr.c */

#include "sched.h"

#include <linux/slab.h>   // kmalloc
#include <linux/kernel.h> // printk for debugging
#include <linux/list.h>   // for list operation (queue)

#define WRR_TIMESLICE (HZ / 100)

void init_wrr_rq(struct wrr_rq *wrr_rq)
{
    INIT_LIST_HEAD(&wrr_rq->queue_head);
    wrr_rq->total_weight = 0;
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    struct wrr_rq *wrr_rq = &rq->wrr;
    struct sched_wrr_entity *wrr_se = &p->wrr;

    LIST_HEAD_INIT(&wrr_se->node);
    list_add_tail(&wrr_se->node, &wrr_rq->queue_head);

    wrr_se->on_rq = 1;
    wrr_se->timeslice = wrr_se->weight * WRR_TIMESLICE;

    /* For the load balancing  */
    wrr_rq->total_weight += wrr_se->weight;
    
    resched_curr(wrr_rq);
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    struct wrr_rq *wrr_rq = &rq->wrr;
    struct sched_wrr_entity *wrr_se = &p->wrr;

    list_del_init(&wrr_se->node);
    wrr_se->on_rq = 0;

    wrr_rq->total_weight -= wrr_se->weight;

    resched_curr(wrr_rq);    
}

static struct task_struct *pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
    struct wrr_rq *wrr_rq = &rq->wrr;
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
    //struct wrr_rq *wrr_rq = &rq->wrr;
    //struct sched_wrr_entity *wrr_se = &prev->wrr;
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
    if(p->policy != SCHED_WRR)
        return;

    struct wrr_rq *wrr_rq = &rq->wrr;
    struct sched_wrr_entity *wrr_se = &p->wrr;
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
    /* Not included in rt.c. included in fair.c  */
    return;
}

const struct sched_class wrr_sched_class = {
    .next = &fair_sched_class,
    .enqueue_task = enqueue_task_wrr,
    .dequeue_task = dequeue_task_wrr,

    .pick_next_task = pick_next_task_wrr,
    .put_prev_task = put_prev_task_wrr,
    
    .task_tick = task_tick_wrr,
    .task_fork = task_fork_wrr,
};
