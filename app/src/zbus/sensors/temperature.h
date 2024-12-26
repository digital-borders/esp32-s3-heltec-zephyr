#ifndef ZBUS_SENSORS_TEMPERATURE_H
#define ZBUS_SENSORS_TEMPERATURE_H

#include <stdint.h>
#include <zephyr/zbus/zbus.h>

/// @brief Temperature sensor message structure
struct sensors_temperature_message {
    float temperature;
};

#endif // ZBUS_SENSORS_TEMPERATURE_H