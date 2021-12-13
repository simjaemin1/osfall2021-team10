#include <linux/syscalls.h>
#include <linux/gps.h>
#include <uapi/asm-generic/errno-base.h>

SYSCALL_DEFINE1(set_gps_location, struct gps_location __user *, loc)
{
    return 0;
}
