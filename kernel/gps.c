#include <linux/syscalls.h>
#include <linux/gps.h>
#include <uapi/asm-generic/errno-base.h>

DEFINE_SPINLOCK(lock);

int valid_location(struct gps_location *loc)
{
  int lat_frac = (0 <= loc->lat_frac) && (loc->lat_frac <= 999999);
  int lng_frac = (0 <= loc->lng_frac) && (loc->lng_frac <= 999999);
  int lat_int = (-90 <= loc->lat_integer) && (loc->lat_integer <= 90);
  int lon_int = (-180 <= loc->lng_integer) && (loc->lng_integer <= 180);
  int acc = (0 <= loc->accuracy);
  
  return lat_frac && lng_frac && lat_int && lon_int && acc;
}

SYSCALL_DEFINE1(set_gps_location, struct gps_location __user *, loc)
{
  if(!valid_location(loc))
  	return EINVAL;
	spin_lock(&lock);
	copy_from_user(&systemloc, loc);
  spin_unlock(&lock);
	return 0;
}




SYSCALL_DEFINE2(get_gps_location, const char __user *pathname, struct gps_location __user *loc)
{


}
