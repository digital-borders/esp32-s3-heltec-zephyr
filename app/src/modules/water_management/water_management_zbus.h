
#ifndef WATER_MANAGEMENT_ZBUS_H
#define WATER_MANAGEMENT_ZBUS_H

#include <zephyr/zbus/zbus.h>

// Channels to publish to
#include "zbus/status/water_pump.h"
ZBUS_CHAN_DECLARE(status_water_pump_channel);

#include "zbus/status/water_valve.h"
ZBUS_CHAN_DECLARE(status_water_valve_channel);

// Channels to listen to
#include "zbus/actuators/water_pump.h"
ZBUS_CHAN_DECLARE(actuators_water_pump_channel);

#include "zbus/actuators/water_valve.h"
ZBUS_CHAN_DECLARE(actuators_water_valve_channel);

// Subscriptions
/**
 * Define an observer of type subscriber for the water management module
 */
ZBUS_SUBSCRIBER_DEFINE(water_management_thread_subscriber, 4);

/**
 * Suscribe to the water pump actuator channel
 */
ZBUS_CHAN_ADD_OBS(actuators_water_pump_channel, water_management_thread_subscriber, 3);

/**
 * Suscribe to the water valve actuator channel
 */
ZBUS_CHAN_ADD_OBS(actuators_water_valve_channel, water_management_thread_subscriber, 3);

#endif // WATER_MANAGEMENT_ZBUS_H