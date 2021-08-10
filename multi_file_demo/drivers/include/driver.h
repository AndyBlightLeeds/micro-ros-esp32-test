#ifndef DRIVERS_DRIVERS_H
#define DRIVERS_DRIVERS_H

/* A minimal example driver interface */

void driver_get_battery_state(int *milli_volts, int *milli_amps);
void driver_set_velocity(int forward_mm_per_second, int rotation_milli_radians);

#endif  // DRIVERS_DRIVERS_H