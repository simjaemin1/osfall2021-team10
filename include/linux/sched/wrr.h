#ifndef _SCHED_WRR_H_
#define _SCHED_WRR_H_

#include <linux/sched.h>

#define WRR_MAX_WEIGHT 20
#define WRR_MIN_WEIGHT  1

#define WRR_TIMESLICE (HZ / 100)
#define WRR_PERIOD    2 * HZ

#define CPU_WITHOUT_WRR 1

#endif
