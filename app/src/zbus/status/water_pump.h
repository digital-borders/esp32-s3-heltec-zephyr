#ifndef ZBUS_STATUS_WATER_PUMP_H
#define ZBUS_STATUS_WATER_PUMP_H

#include <stdint.h>
#include <stdbool.h>

/// @brief Water pump status message structure
struct status_water_pump_message {
    bool running;
};

#endif // ZBUS_STATUS_WATER_PUMP_H