#include <stdio.h>

#include "drivers.h"

void driver_get_battery_state(int *milli_volts, int *milli_amps) {
  *milli_volts = 3812;
  *milli_amps = 123;
}

void driver_set_velocity(int forward_mm_per_second,
                         int rotation_milli_radians) {
  printf("Driver: %d forward, %d rotation\n", forward_mm_per_second,
         rotation_milli_radians);
}
