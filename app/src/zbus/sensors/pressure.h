#ifndef ZBUS_SENSORS_PRESSURE_H
#define ZBUS_SENSORS_PRESSURE_H

#include <stdint.h>
#include <zephyr/zbus/zbus.h>

/// @brief Pressure sensor message structure
struct sensors_pressure_message {
	float pressure;
};

#endif // ZBUS_SENSORS_PRESSURE_H