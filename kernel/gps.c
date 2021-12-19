#include <linux/syscalls.h>
#include <linux/gps.h>
#include <uapi/asm-generic/errno-base.h>
#include <uapi/linux/fcntl.h>
#include <linux/namei.h>
#include <linux/dcache.h>
#include <linux/slab.h>

#define CORRECTION 1000000LL
#define INT(x) (x / 1000000LL)
#define FRAC(x) (x % 1000000LL)

DEFINE_SPINLOCK(gps_lock);

struct gps_location systemloc;

typedef struct _fixed {
	long long int int_f;
  long long int frac_f;
} fixed;

/*********************************************************************
 Assume that the Earth is perfect sphere with radius 6371km (6371000m)
 the distance of 1 degree of latitude is R * PI * 180 = 111.195km
**********************************************************************/

fixed PI = { 3, 141592 };
fixed R  = { 6371000, 0 };
fixed DIST = { 111195, 0 };
fixed D2R = { 0, 17453 }; // degree to radian -> PI/180

fixed neg(fixed val)
{
  fixed ret;
  ret.int_f = -val.int_f - 1;
  ret.frac_f = -val.frac_f + CORRECTION;
  return ret;
}

fixed add(fixed num1, fixed num2)
{
	long long int intint, fracfrac;
  fixed ret;

  intint = num1.int_f + num2.int_f;
  fracfrac = num1.frac_f + num2.frac_f;

  ret.int_f = intint + INT(fracfrac);
  ret.frac_f = FRAC(fracfrac);

  return ret;
}

fixed sub(fixed num1, fixed num2)
{
	long long int intint, fracfrac;
  fixed ret;

  intint = num1.int_f - num2.int_f;
  fracfrac = num1.frac_f - num2.frac_f;

  if(fracfrac < 0) {
  	intint--;
    fracfrac += CORRECTION;
  }

  ret.int_f = intint + INT(fracfrac);
  ret.frac_f = FRAC(fracfrac);

	return ret;
}

fixed mul(fixed num1, fixed num2)
{
	fixed ret;
  long long int intint, intfrac, fracint, fracfrac, midfrac;
  long long integer, fractional;

  intint = num1.int_f * num2.int_f;
  intfrac = num1.int_f * num2.frac_f;
  fracint = num1.frac_f * num2.int_f;
  fracfrac = num1.frac_f * num2.frac_f;

  midfrac = intfrac + fracint + INT(fracfrac);

  ret.int_f = intint + INT(midfrac);
  ret.frac_f = FRAC(midfrac);

  if(ret.frac_f < 0) {
  	ret.int_f--;
    ret.frac_f += CORRECTION;
  }

	return ret;
}

fixed pow_f(fixed num, int iter)
{
	if(iter == 1) {
  	return num;
  }

  return mul(num, pow_f(num, iter - 1));
}

fixed div(fixed num, long long int div)
{
	fixed ret;

  /**************************************************************************
   It can be possible because we use it for cosine function,
   and the value for radian is between -2pi and 2pi -> relatively small
   -> enough to use rough formula of division. (No possibilities of overflow)
  ***************************************************************************/

  long long int val = (num.int_f * CORRECTION + num.frac_f) / div;
  ret.int_f = INT(val);
  ret.frac_f = FRAC(val);
  if(ret.frac_f < 0) {
      ret.int_f--;
      ret.frac_f += CORRECTION;
  }

  return ret;
}

long long int factorial(long long int num)
{
	if(num == 2) {
  	return 2;
  }

  return num * factorial(num - 1);
}

fixed avg(fixed num1, fixed num2)
{
	fixed sum, ret;
  long long int val;

	/* It works only for degree (to get average latitude) */
	sum = add(num1, num2);
  val = (sum.int_f * CORRECTION + sum.frac_f) / 2;

  ret.int_f = INT(val);
  ret.frac_f = FRAC(val);

  return ret;
}

fixed cos_f(fixed deg)
{
	// first we need to convert degree value to radian
  fixed rad, ret, val;
  int i;

  rad = mul(deg, D2R); // deg * pi / 180
  ret.int_f = 0;
  ret.frac_f = 0;

  for(i = 0; i < 8; i++) {
  	if(i == 0) {
    	// first iteration, just 1
    	ret.int_f = 1;
    }
    else {
    	/*********************
       1. x^i / fact(i)
       2. multiply (-1)^i
       3. ret += val (sigma)
      **********************/
    	val = div(pow_f(rad, 2 * i), (int)factorial((long long int)(2 * i)));
      if(i % 2 == 1) {
      	// multiply (-1)^i
      	val = neg(val);
      }
      ret = add(ret, val);
    }
  }
  return ret;
}

long long int get_dist(struct gps_location* loc1, struct gps_location* loc2)
{
	long long int dist;
  fixed lat1, lng1, lat2, lng2;
  fixed difflat, difflng;
  fixed dlat, dlng;
  fixed latavg;
  fixed x, y;
  fixed d;

  lat1.int_f = (long long int)loc1->lat_integer;
  lat1.frac_f = (long long int)loc1->lat_fractional;
  lng1.int_f = (long long int)loc1->lng_integer;
  lng1.frac_f = (long long int)loc1->lng_fractional;
  lat2.int_f = (long long int)loc2->lat_integer;
  lat2.frac_f = (long long int)loc2->lat_fractional;
  lng2.int_f = (long long int)loc2->lng_integer;
  lng2.frac_f = (long long int)loc2->lng_fractional;

  difflat = sub(lat1, lat2);
  difflng = sub(lng1, lng2);

  dlat = (difflat.int_f < 0) ? neg(difflat) : difflat;
  dlng = (difflng.int_f < 0) ? neg(difflng) : difflng;

  latavg = avg(lat1, lat2);

  y = mul(DIST, dlat); // 111km * (lat1 - lat2)
  x = mul(cos_f(latavg), mul(DIST, dlng));

  d = add(mul(x, x), mul(y, y));

  /************************************************************************
   only returns integer value because we will compare dist with accuracy(m)
   cast accuracy to long long int and compare with this value
  *************************************************************************/

  dist = d.int_f;
	return dist;
}

int valid_location(struct gps_location *loc)
{
  int lat_fractional = (0 <= loc->lat_fractional) && (loc->lat_fractional <= 999999);
  int lng_fractional = (0 <= loc->lng_fractional) && (loc->lng_fractional <= 999999);
  int lat_int = (-90 <= loc->lat_integer) && (loc->lat_integer <= 90);
  int lng_int = (-180 <= loc->lng_integer) && (loc->lng_integer <= 180);
  int acc = (0 <= loc->accuracy);
  
  return lat_fractional && lng_fractional && lat_int && lng_int && acc;
}

SYSCALL_DEFINE1(set_gps_location, struct gps_location __user *, loc)
{
	struct gps_location k_loc;
	copy_from_user(&k_loc, loc, sizeof(struct gps_location));
	if(!valid_location(&k_loc))
		return -EINVAL;
	spin_lock(&gps_lock);
	systemloc.lat_integer = k_loc.lat_integer;
	systemloc.lat_fractional = k_loc.lat_fractional;
	systemloc.lng_integer = k_loc.lng_integer;
	systemloc.lng_fractional = k_loc.lng_fractional;
	systemloc.accuracy = k_loc.accuracy;
	spin_unlock(&gps_lock);
	return 0;
}

int LocationCompare(struct gps_location *locA, struct gps_location *locB)
{
    long long int accuracy_sum, accuracy_square, dist;

    accuracy_sum = (long long int)locA->accuracy + (long long int)locB->accuracy;
    accuracy_square = accuracy_sum * accuracy_sum;
    dist = get_dist(locA, locB);

	return (dist <= accuracy_square);
}


SYSCALL_DEFINE2(get_gps_location, const char __user *, pathname, struct gps_location __user *, loc)
{
	struct path path;
	unsigned int lookup_flags = LOOKUP_FOLLOW;
	struct gps_location k_loc;
	long path_length;
	char * k_pathname;

	if(path_length = strnlen_user(pathname, 1000L))
	{
		printk("ERROR: invalid path length\n");
		return -EINVAL;
	}

	if(k_pathname = (char*)kmalloc(path_length, GFP_KERNEL))
	{
		printk("ERROR: kmalloc error\n");
		return -EINVAL;
	}

	if(strncpy_from_user(k_pathname, pathname, path_length))
	{
		printk("ERROR: strncpy_user error\n");
		kfree(k_pathname);
		return -EINVAL;
	}

	if(user_path_at_empty(AT_FDCWD, k_pathname, lookup_flags, &path, NULL))
	{
		printk("ERROR: no such pathname\n");
		kfree(k_pathname);
		return -EFAULT;	
	}

	struct inode *file_inode = d_inode(path.dentry);
	if(!file_inode->i_op->get_gps_location)
	{
		printk("ERROR: no gps-coordinate system embedded\n");
		kfree(k_pathname);
		return -ENODEV;
	}
	file_inode->i_op->get_gps_location(file_inode, &k_loc);
	
	spin_lock(&gps_lock);
	if(!LocationCompare(&k_loc, &systemloc)) {
		spin_unlock(&gps_lock);
		kfree(k_pathname);
		return -EACCES;
	}
	spin_unlock(&gps_lock);
	
	copy_to_user(loc, &k_loc, sizeof(struct gps_location));
	kfree(k_pathname);
	return 0;
}
