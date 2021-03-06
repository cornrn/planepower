#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "bitbang_spi.h"

// Program to run on the armbone (probably over ssh)
// to see if the beaglebone can read data from the line angle sensors.

int main()
{
	printf("Intializing GPIO pins...\n");
	bitbang_init();

	float azimuth_radians,elevation_radians;

	while(1)
	{
		read_angle_sensors(&azimuth_radians, &elevation_radians);
#if 1
		// Print the values in radians
		printf("AZIMUTH: %f ELEVATION: %f\n",azimuth_radians, elevation_radians);
#endif
#if 1
		plot_two_angles(azimuth_radians, elevation_radians);
		usleep(300000);
#endif

		printf("\n");
		usleep(500);
	}
	bitbang_close();
}
