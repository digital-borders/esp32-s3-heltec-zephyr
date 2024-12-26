#ifndef ZBUS_ACTUATORS_WATER_PUMP_H
#define ZBUS_ACTUATORS_WATER_PUMP_H

#include <stdbool.h>

/// @brief Water pump actuator message structure
struct actuators_water_pump_message {
	bool run;
	uint32_t time_remaining;
};

#endif // ZBUS_ACTUATORS_WATER_PUMP_H