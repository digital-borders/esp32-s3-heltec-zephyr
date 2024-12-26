#ifndef ZBUS_STATUS_MQTT_H
#define ZBUS_STATUS_MQTT_H

#include <stdint.h>
#include <stdbool.h>

/// @brief Mqtt status message structure
struct status_mqtt_message {
    bool connected;
};

#endif // ZBUS_STATUS_MQTT_H