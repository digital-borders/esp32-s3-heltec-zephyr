#ifndef ZBUS_CONFIG_SENSORS_H
#define ZBUS_CONFIG_SENSORS_H

#include <stdint.h>

/// @brief Senosrs configuration message structure
struct config_sensors_message {
	uint32_t temperature_poll_interval_ms;
	uint32_t humidity_poll_interval_ms;
	uint32_t pressure_poll_interval_ms;
	uint32_t water_level_poll_interval_ms;
	uint32_t battery_poll_interval_ms;
};

#endif // ZBUS_CONFIG_SENSORS_H