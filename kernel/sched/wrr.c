/* OSFALL2021 TEAM 10 PROJ2 / KOO HAN BIN & LEE SEUNG HYOUN */

/* kernel/sched/wrr.c */

#include "sched.h"

#include <linux/slab.h>


const struct sched_class wrr_sched_class = {
	.next									= &fair_sched_class,
	.enqueue_task					= enqueue_task_wrr,
	.dequeue_task 				= dequeue_task_wrr,
	.yield_task						= yield_task_wrr,
  .yield_to_task 				= yield_to_task_wrr,
  
  //.check_preempt_curr 	= check_preempt_
  
	.pick_next_task				= pick_next_task_wrr,
  .put_prec_task				= put_prev_task_wrr,
  
  #ifdef CONFIG_SMP
  
  
  
  #endif
  
  .set_curr_task				= set_curr_task_wrr,
};

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags) 
{
}

static void dequeue_task_wrr(struct rq *rp, struct task_struct *p, int flags) 
{
}

static void yield_task_wrr(struct rp *rp) 
{
}

static void yield_to_task_wrr() 
{
}

static void pick_next_task_wrr() 
{
}

static void put_prev_task_wrr()
{
}




