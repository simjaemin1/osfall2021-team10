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
    wrr_rq.total_weight = 0;
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    struct wrr_rq *wrr_rq = &rq->wrr_rq;
    struct sched_wrr_entity *wrr_se = &p->wrr;
    struct list_head head = wrr_rq->queue_head;

    LIST_HEAD_INIT(&wrr_se->node);
    list_add_tail(&wrr_se->node, &head);

    wrr_se->on_rq = 1;
    wrr_se->timeslice = wrr_se->weight * WRR_TIMESLICE;

    // It will print information of runqueue -> debugging purpose        
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    struct wrr_rq *wrr_rq;
    struct sched_wrr_entity *wrr_se = &p->wrr;
}

static void pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
    struct wrr_rq *wrr_rq = &rq->wrr;
    struct sched_wrr_entity *wrr_se;
    struct task_struct *p;
    int new_tasks;
}

static void put_prev_task_fair(struct rq *rq, struct task_struct *prev)
{
    struct wrr_rq *wrr_rq;
    struct sched_wrr_entity *wrr_se = &prev->wrr;
}

struct void 

static void set_curr_task_wrr(struct rq *rq)
{
    struct sched_wrr_entity *wrr_se = &rq->curr->wrr;
}

static void task_tick_wrr()
{
}

const struct sched_class wrr_sched_class = {
    .next = &fair_sched_class,
    .enqueue_task = enqueue_task_wrr,
    .dequeue_task = dequeue_task_wrr,

    .pick_next_task = pick_next_task_wrr,
    .put_prev_task = put_prev_task_wrr,

    .set_curr_task = set_curr_task_wrr,
    
    .task_tick = task_tick_wrr,
};
