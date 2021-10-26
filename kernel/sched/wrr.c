/* OSFALL2021 TEAM 10 PROJ2 / KOO HAN BIN & LEE SEUNG HYOUN */

/* kernel/sched/wrr.c */

#include "sched.h"

#include <linux/slab.h>

void init_wrr_rq(struct wrr_rq *wrr_rq)
{
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    struct wrr_rq *wrr_rq;
    struct sched_entity *se = &p->se;
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    struct wrr_rq *wrr_rq;
    struct sched_entity *se = &p->se;
}

static void pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
    struct wrr_rq *wrr_rq = &rq->wrr;
    struct sched_entity *se;
    struct task_struct *p;
    int new_tasks;
}

static void put_prev_task_fair(struct rq *rq, struct task_struct *prev)
{
    struct wrr_rq *wrr_rq;
    struct sched_entity *se = &prev->se;
}

struct void 

static void set_curr_task_wrr(struct rq *rq)
{
    struct sched_entity *se = &rq->curr->se;
}

const struct sched_class wrr_sched_class = {
    .next = &fair_sched_class,
    .enqueue_task = enqueue_task_wrr,
    .dequeue_task = dequeue_task_wrr,

    .pick_next_task = pick_next_task_wrr,
    .put_prev_task = put_prev_task_wrr,

    .set_curr_task = set_curr_task_wrr,
};
