#include <zephyr/zbus/zbus.h>

#include "water_pump.h"
#include "water_valve.h"
#include "buzzer.h"

// Water pump actuator channel
ZBUS_CHAN_DEFINE(
    actuators_water_pump_channel,
    struct actuators_water_pump_message,
    NULL,
    NULL,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT(
        .run = false
    ));

// Water valve actuator channel
ZBUS_CHAN_DEFINE(
    actuators_water_valve_channel,
    struct actuators_water_valve_message,
    NULL,
    NULL,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT(
        .open = false
    ));

// Buzzer actuator channel
ZBUS_CHAN_DEFINE(
    actuators_buzzer_channel,
    struct actuators_buzzer_message,
    NULL,
    NULL,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT(
        .onOff = false
    ));