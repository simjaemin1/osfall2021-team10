### OSFALL2021 TEAM10 PROJ2


## 0. How to build kernel
다음과 같은 directory 상황에서 tizen-5.0-rpi3 폴더를 kernel path라고 가정한다.
tizen-kernel
├── tizen-5.0-rpi3
├── tizen-image
└── mnt_dir

우선 -----------.

## 1. High level implementation - WRR
#### 1.1 Define SCHED_WRR
<kernel path>/include/uapi/linux/sched.h에 SCHED_WRR를 7로 추가해주었다.

#### 1.2 Define sched_wrr_entity
실행되는 각 task마다 WRR policy에 사용될 sched_wrr_entity struct가 필요하여 정의해주었다.
include/linux/sched.h에 task_struct와 같이 정의되어있다.

struct sched_wrr_entity는 다음과 같은 변수와 함께 정의되어있다.
1. struct list_head node
    wrr_rq에 사용될 struct list_head이다.
2. unsigned int weight
    task의 timeslice와 load_balancing에 사용되는 weight parameter이다.
3. unsigned int timeslice
    task가 가진 고유의 timeslice이다. 다 사용할 경우 runqueue의 가장 마지막으로 이동    한다. weight*10ms의 값을 가진다.
4. unsigned short on_rq
    task가 wrr runqueue에 속해있는지에 대한 값이다. 1이면 wrr runqueue에 속해있고,
    0이면 그렇지 않는 것을 뜻한다.
5. !!!!!!!!!이후 추가 필요!!!!!!!!!!!!1

이후 task_struct가 정의된 부분에 각 task마다 wrr entity를 가질 수 있다는 것을
알려주기 위해 entity를 구조체 변수로 추가해주었다.
`struct sched_wrr_entity wrr`

#### 1.3 Systemcall : sched_setweight, shced_getweight
다른 process에서 systemcall로 pid를 통해 특정 process가 가진 wrr policy에 대한 weight값을 get하거나 set하게 위해 두 systemcall을 추가한다.

* include/linux/syscalls.h
    아래 두 줄을 추가한다.
    `asmlinkage long sys_sched_setweight(pid_t pid, int weight)`
    `asmlinkage long sys_sched_getweight(pid_t pid)`
* arch/arm/tools/syscalls.tbl
    아래 두 줄을 추가한다.
    `398 common sched_setweight sys_sched_setweight`
    `399 common sched_getweight sys_sched_getweight`
* arch/arm64/include/asm/unistd.h
    `#define __NR_compat_syscalls   400`
    으로 수정한다.
* arch/arm64/include/asm/unistd.h
    아래 네 줄을 추가한다.
    `#define __NR_sched_setweight 398`
    `__SYSCALL(__NR_sched_setweight, sys_sched_setweight)`
    `#define __NR_sched_getweight 399`
    `__SYSCALL(__NR_sched_getweight, sys_sched_getweight)`

사전작업 후 kernel/sched/core.c에
    `SYSCALL_DEFINE2(sched_setweight, pid_t, pid, int, weight)`
    `SYSCALL_DEFINE1(sched_getweight, pid_t, pid)`
    두 함수를 정의해주었다.

#### 1.4 Define wrr_rq
kernel/sched/sched.h에는 현재 task들이 돌아가고 있는 runqueue에 대한 구조체인 struct rq가 정의되어 있고, 각 scheduler policy마다  runqueue가 따로 정의되어있다.
여기에 나중에 WRR policy가 되었을 때 사용하기 위한 wrr_rq를 정의하고 struct rq 내부에 추가해준다.

struct wrr_rq에는 다음과 같은 변수와 함께 정의되어있다.
1. struct list_head queue_head
    이 변수는 list로 구현된 runqueue의 head 역할을 한다. list.h의 api를 이용해 구현된 list의 head는 빈 head로 취급한다. 이 queue_head의 뒤부터 struct sched_wrr_entity를 이어 queue를 구성한다.
2. unsigned int total_weight
    load_balancing에 사용하기 위해 runqueue에 저장된 모든 task의 weight에 대한 합의 변수를 가지고있다.
3. !!!!!!!!추후 추가 필요!!!!!!!!!!!!!!

#### 1.5 kernel/sched/wrr.c
먼저 sturct shced_wrr_class를 정의하고 내부에 함수들을 지정해주어 후에 kernel/sched/core.c에서 사용하는 여러 함수들을 WRR policy에서도 써줄 수 있도록 한다.
/kernel/sched/rt.c에 정의된 sched_rt_class의 .next = &sched_wrr_class로 rt policy 다음에 wrr policy인 것을 명시해준다.

이번 프로젝트에서 필요한 함수는 다음과 같다고 판단하였다.
1. .enqueue_task = enqueue_task_wrr
    scheduler에서 특정 task가 policy에 맞는 runqueue에 enqueue하기 위한 함수이다.
    list로 queue를 구현했으므로 list의 tail부분에 task를 추가한다. 이후 task의 timeslice를 weight에 맞게 정해주고, task가 rq에 들어왔으므로 on_rq를 1로 만들어준다. 또한 wrr_rq의 total_weight도 조정해준다.
    이후 resched_curr 함수를 call 한다.
2. .dequeue_task = dequeue_task_wrr
    task의 실행이 완료되었을때 dequeue하기 위한 함수이다.
    해당 node를 runqueue에서 제거하고, on_rq = 0으로 assign해준 후에 total_weight도 감소시킨다.
    이후 resched_curr 함수를 call 한다.
3. .pick_next_task = pick_next_task_wrr
    scheduler에서 다음 실행할 task를 찾는 함수이다. WRR policy에선 queue의 front가 항상 실행되어야 하므로 그것을 return해준다.
4. .put_prev_task = put_prev_task_wrr
5. .task_tick = task_tick_wrr
    task_tick 함수는 kernel이 정해진 시간마다 scheduler_tick 함수를 call 할때마다 그 내부에서 call되는 함수이다. 현재 runqueue의 가장 front에서 실제로 cpu를 차지하여 실행되고 있는 task의 timeslice를 관리한다. timeslice를 모두 소모한 task는 dequeue 후에 weight에 따라 다시 timeslice를 채워 enqueue한다.
6. .task_fork = task_fork_wrr
    task_fork 함수는 parent로부터 fork된 child process에 대해 지정해주는 함수이다.
7. init_wrr_rq
    sched_init 함수에서 scheduler initialization을 할 때 wrr_rq를 초기화 해주기 위해 call하는 함수이다.
    먼저 wrr_rq->queue_head를 초기화 해주고 total_weight 변수도 0으로 초기화 해준다.

#### 1.6 kernel/sched/core.c
-----core.c에서 어떤어떤거 한다 간략한 설명-----

다음과 같은 부분을 수정하였다.
1. SYSCALL_DEFINE2(sched_setweight, pid_t, pid, int, weight)
    먼저 pid 값이 0보다 작거나, weight을 20보다 크거나 또는 1보다 작은 valid하지 않은 argument를 전달한 경우 함수를 중단하였다.
    이후 rcu_read_lock을 통해 중간에 값이 변하는 것을 방지하고, getuid()를 통해 이 함수를 call한 process의 privilege level을 체크한다. admin mode가 아닌 경우 error msg를 남기고 return한다. admin인 경우 pid에 맞는 task의 wrr entity에 접근하여 weight값을 바꿔준다.
    이후 unlock한 후 그 weight값을 return한다.

    !!!!!!!!!!! 여기에 pid에 맞는 task가 없는 경우와, SCHED_WRR가 아닌 경우도 넣어줘야함!!!!!!
2. SYSCALL_DEFINE1(sched_getweight, pid_t, pid)
    pid 값이 0보다 작은 경우 유효한 pid가 아니므로 함수를 중단하였다.
    rcu_read_lock()을 해주고 pid에 따른 task_struct를 가져온 다음 그 weight값을 return하였다.
    !!!!!!!!!! 여기에 pid에 맞는 task가 없는 경우와, SCHED_WRR가 아닌 경우도 넣어줘야함!!!!!!!!
3. void sched_init(void)
    scheduler 초기화 단계에서 init_wrr_rq(&rq->wrr) 를 통해 wrr_rq를 초기화해준다.
4. static void sched_fork
    fork된 task의 scheduling을 처리하는 부분에 대한 함수이다. fork된 task의 wrr entity 구조체의 변수들을 초기화해준다.

