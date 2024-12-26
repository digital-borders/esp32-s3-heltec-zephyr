#ifndef ZBUS_STATUS_WATER_VALVE_H
#define ZBUS_STATUS_WATER_VALVE_H

#include <stdint.h>
#include <stdbool.h>

/// @brief Water valve status message structure
struct status_water_valve_message {
    bool opened;
};

#endif // ZBUS_STATUS_WATER_VALVE_H