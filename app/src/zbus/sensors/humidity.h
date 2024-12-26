#ifndef ZBUS_SENSORS_HUMIDITY_H
#define ZBUS_SENSORS_HUMIDITY_H

#include <stdint.h>
#include <zephyr/zbus/zbus.h>

/// @brief Humidity sensor message structure
struct sensors_humidity_message {
    float humidity;
};

#endif // ZBUS_SENSORS_HUMIDITY_H