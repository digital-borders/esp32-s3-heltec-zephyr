#ifndef ZBUS_ACTUATORS_WATER_VALVE_H
#define ZBUS_ACTUATORS_WATER_VALVE_H

#include <stdbool.h>

/// @brief Water valve actuator message structure
struct actuators_water_valve_message {
    bool open;
};

#endif // ZBUS_ACTUATORS_WATER_VALVE_H