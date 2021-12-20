#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/gps.h>

#define SET_GPS_LOCATION 398
#define CORRECTION 1000000

void set_location(struct gps_location *loc)
{
	int error;
	error = syscall(SET_GPS_LOCATION, loc);
	if(error){
		printf("error code: %d\n", error);
	}
}

int main(int argc, char *argv[])
{
    if(argc != 6)
    {
        printf("Usage: ./gpsupdate lat_integer lat_fractional lng_integer lng_fractional accuracy\n");
	return 0;
    }
    
    struct gps_location loc = { atoi(argv[1]), atoi(argv[2]), 
        atoi(argv[3]), atoi(argv[4]), atoi(argv[5]) };

	set_location(&loc);	
	return 0;
}


