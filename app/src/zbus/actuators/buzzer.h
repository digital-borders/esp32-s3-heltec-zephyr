#ifndef ZBUS_ACTUATORS_BUZZER_H
#define ZBUS_ACTUATORS_BUZZER_H

#include <stdbool.h>

/// @brief Buzzer actuator message structure
struct actuators_buzzer_message {
    bool onOff;
};

#endif // ZBUS_ACTUATORS_BUZZER_H