#ifndef ZBUS_SENSORS_WATER_LEVEL_H
#define ZBUS_SENSORS_WATER_LEVEL_H

#include <stdint.h>
#include <zephyr/zbus/zbus.h>

/// @brief Water level sensor message structure
struct sensors_water_level_message {
	int percentage;
};

#endif // ZBUS_SENSORS_WATER_LEVEL_H