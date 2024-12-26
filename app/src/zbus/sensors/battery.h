#ifndef ZBUS_SENSORS_BATTERY_H
#define ZBUS_SENSORS_BATTERY_H

#include <stdint.h>
#include <zephyr/zbus/zbus.h>

/// @brief Battery sensor message structure
struct sensors_battery_message {
	int percentage;
};

#endif // ZBUS_SENSORS_BATTERY_H