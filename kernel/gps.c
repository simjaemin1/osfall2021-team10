#include <linux/syscalls.h>
#include <linux/gps.h>
#include <uapi/asm-generic/errno-base.h>
#include <uapi/linux/fcntl.h>
#include <linux/namei.h>
#include <linux/dcache>

DEFINE_SPINLOCK(lock);

int valid_location(struct gps_location *loc)
{
  int lat_frac = (0 <= loc->lat_frac) && (loc->lat_frac <= 999999);
  int lng_frac = (0 <= loc->lng_frac) && (loc->lng_frac <= 999999);
  int lat_int = (-90 <= loc->lat_integer) && (loc->lat_integer <= 90);
  int lng_int = (-180 <= loc->lng_integer) && (loc->lng_integer <= 180);
  int acc = (0 <= loc->accuracy);
  
  return lat_frac && lng_frac && lat_int && lon_int && acc;
}

SYSCALL_DEFINE1(set_gps_location, struct gps_location __user *, loc)
{
	struct gps_location k_loc;
	copy_from_user(&k_loc, loc, sizeof(struct gps_location));
	if(!valid_location(k_loc))
		return -EINVAL;
	spin_lock(&lock);
	systemloc.lat_integer = k_loc.lat_integer;
	systemloc.lat_frac = k_loc.lat_frac;
	systemloc.lng_integer = k_loc.lng_integer;
	systemloc.lng_frac = k_loc.lng_frac;
	systemloc.accuracy = k_loc.accuracy;
	spin_unlock(&lock);
	return 0;
}




SYSCALL_DEFINE2(get_gps_location, const char __user *pathname, struct gps_location __user *loc)
{
	struct path path;
	unsigned int lookup_flags = LOOKUP_FOLLOW;
	struct gps_location k_loc;
	long path_length;
	char * k_pathname;

	if(path_lenth = strlen_user(pathname))
	{
		printk("ERROR: invalid path length\n");
		return -EINVAL;
	}

	if(k_pathname = (char*)kmalloc(path_length, GFP_KERNEL))
	{
		printk("ERROR: kmalloc error\n");
		return -EINVAL;
	}

	if(strncpy_user(k_pathname, pathname, path_length))
	{
		printk("ERROR: strncpy_user error\n");
		return -EINVAL;
	}

	if(user_path_at_empty(AT_FDCWD, k_pathname, lookup_flags, &path, NULL))
	{
		printk("ERROR: no such pathname\n");
		return -EFAULT;	
	}

	struct inode *file_inode = d_inode(path.dentry);
	if(!file_inode->i_op->get_gps_location)
	{
		printk("ERROR: no gps-coordinate system embedded\n");
		return -ENODEV;
	}
	file_inode->i_op->get_gps_location(file_inode, &k_loc);
	
	spin_lock(&gps_lock);
	if(!LocationCompare(&k_loc, systemloc)) {
		spin_unlock(&gps_lock);
		return -EACCES;
	}
	spin_unlock(&gps_lock);
	
	copy_to_user(loc, &k_loc, sizeof(struct gps_location));
	return 0;
}
