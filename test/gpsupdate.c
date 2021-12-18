#include <stdio.h>
#include <sys.syscall.h>
#include <linux/gps.h>

#define SET_GPS_LOCATION 398


int main()
{
	struct gps_location loc={1,1,1,1,1};//change

	change_current_location(&loc, 1, 2, 1);
	set_location(&loc);	
	return 0;
}

void change_current_location(struct gps_location *loc, int lat_change, int lng_change, int newaccuracy)
{	
	//TODO
}

void set_location(struct gps_location *loc)
{
	int error;
	error = syscall(SET_GPS_LOCATION, loc);
	if(error){
		printf("error code: %d\n", error)
		return -1;
	}
}
