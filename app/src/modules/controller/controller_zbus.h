#include <zephyr/zbus/zbus.h>

#include "zbus/config/farming.h"
ZBUS_CHAN_DECLARE(config_farming_str_channel);

#include "zbus/sensors/rtc.h"
ZBUS_CHAN_DECLARE(sensors_rtc_channel);

#include "zbus/status/water_pump.h"
ZBUS_CHAN_DECLARE(status_water_pump_channel);

#include "zbus/status/water_valve.h"
ZBUS_CHAN_DECLARE(status_water_valve_channel);

#include "zbus/sensors/humidity.h"
ZBUS_CHAN_DECLARE(sensors_humidity_channel);

#include "zbus/sensors/temperature.h"
ZBUS_CHAN_DECLARE(sensors_temperature_channel);

#include "zbus/sensors/pressure.h"
ZBUS_CHAN_DECLARE(sensors_pressure_channel);

#include "zbus/sensors/co.h"
ZBUS_CHAN_DECLARE(sensors_co_channel);

#include "zbus/sensors/water_level.h"
ZBUS_CHAN_DECLARE(sensors_water_level_channel);

#include "zbus/actuators/water_pump.h"
ZBUS_CHAN_DECLARE(actuators_water_pump_channel);

#include "zbus/actuators/water_valve.h"
ZBUS_CHAN_DECLARE(actuators_water_valve_channel);

// Define the subscribers to be used by the controller module

/// @brief Define an observer of type subscriber with a queue size of 15
ZBUS_SUBSCRIBER_DEFINE(controller_thread_subscriber, 15);

// Subscribe to the rtc channel
ZBUS_CHAN_ADD_OBS(sensors_rtc_channel, controller_thread_subscriber, 3);

// Suscribe to the config user channel
ZBUS_CHAN_ADD_OBS(config_farming_str_channel, controller_thread_subscriber, 3);

// Suscribe to the humidity sensor channel
ZBUS_CHAN_ADD_OBS(sensors_humidity_channel, controller_thread_subscriber, 3);

// Subscribe to the temperature sensor channel
ZBUS_CHAN_ADD_OBS(sensors_temperature_channel, controller_thread_subscriber, 3);

// Subscribe to the pressure sensor channel
ZBUS_CHAN_ADD_OBS(sensors_pressure_channel, controller_thread_subscriber, 3);

// Subscribe to the co sensor channel
ZBUS_CHAN_ADD_OBS(sensors_co_channel, controller_thread_subscriber, 3);

// Subscribe to the water level sensor channel
ZBUS_CHAN_ADD_OBS(sensors_water_level_channel, controller_thread_subscriber, 3);

// Subscribe to the Water pump status channel
ZBUS_CHAN_ADD_OBS(status_water_pump_channel, controller_thread_subscriber, 3);

// Subscribe to the Water valve status channel
ZBUS_CHAN_ADD_OBS(status_water_valve_channel, controller_thread_subscriber, 3);