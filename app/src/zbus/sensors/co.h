#ifndef ZBUS_SENSORS_CO_H
#define ZBUS_SENSORS_CO_H

#include <stdint.h>
#include <zephyr/zbus/zbus.h>

/// @brief Temperature sensor message structure
struct sensors_co_message {
	float co;
};

#endif // ZBUS_SENSORS_CO_H