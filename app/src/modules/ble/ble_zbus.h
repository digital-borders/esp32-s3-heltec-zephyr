#include <zephyr/zbus/zbus.h>

// In order to use a Zbus channel, you need to declare it first.

#include "zbus/status/wifi.h"
ZBUS_CHAN_DECLARE(status_wifi_channel);

#include "zbus/status/ble.h"
ZBUS_CHAN_DECLARE(status_ble_channel);

#include "zbus/config/farming.h"
ZBUS_CHAN_DECLARE(config_farming_str_channel);

#include "zbus/status/water_pump.h"
ZBUS_CHAN_DECLARE(status_water_pump_channel);

#include "zbus/status/water_valve.h"
ZBUS_CHAN_DECLARE(status_water_valve_channel);

#include "zbus/config/wifi.h"
ZBUS_CHAN_DECLARE(config_wifi_channel);

#include "zbus/config/user.h"
ZBUS_CHAN_DECLARE(config_user_channel);

#include "zbus/info/device.h"
ZBUS_CHAN_DECLARE(info_device_channel);

#include "zbus/sensors/humidity.h"
ZBUS_CHAN_DECLARE(sensors_humidity_channel);

#include "zbus/sensors/temperature.h"
ZBUS_CHAN_DECLARE(sensors_temperature_channel);

#include "zbus/sensors/pressure.h"
ZBUS_CHAN_DECLARE(sensors_pressure_channel);

#include "zbus/sensors/co.h"
ZBUS_CHAN_DECLARE(sensors_co_channel);

#include "zbus/sensors/battery.h"
ZBUS_CHAN_DECLARE(sensors_battery_channel);

#include "zbus/sensors/water_level.h"
ZBUS_CHAN_DECLARE(sensors_water_level_channel);

#include "zbus/actuators/water_pump.h"
ZBUS_CHAN_DECLARE(actuators_water_pump_channel);

#include "zbus/actuators/water_valve.h"
ZBUS_CHAN_DECLARE(actuators_water_valve_channel);

// Define the subscribers to be used by the BLE module

/// @brief Define an observer of type subscriber with a queue size of 15
ZBUS_SUBSCRIBER_DEFINE(ble_thread_subscriber, 15);

// Suscribe to the app info channel to get the device UUID
ZBUS_CHAN_ADD_OBS(info_device_channel, ble_thread_subscriber, 3);

// Suscribe to the farming configuration channel
ZBUS_CHAN_ADD_OBS(config_farming_str_channel, ble_thread_subscriber, 3);

// Suscribe to the config user channel
ZBUS_CHAN_ADD_OBS(config_user_channel, ble_thread_subscriber, 3);

// Suscribe to the humidity sensor channel
ZBUS_CHAN_ADD_OBS(sensors_humidity_channel, ble_thread_subscriber, 3);

// Subscribe to the temperature sensor channel
ZBUS_CHAN_ADD_OBS(sensors_temperature_channel, ble_thread_subscriber, 3);

// Subscribe to the pressure sensor channel
ZBUS_CHAN_ADD_OBS(sensors_pressure_channel, ble_thread_subscriber, 3);

// Subscribe to the CO sensor channel
ZBUS_CHAN_ADD_OBS(sensors_co_channel, ble_thread_subscriber, 3);

// Subscribe to the battery sensor channel
ZBUS_CHAN_ADD_OBS(sensors_battery_channel, ble_thread_subscriber, 3);

// Subscribe to the water level sensor channel
ZBUS_CHAN_ADD_OBS(sensors_water_level_channel, ble_thread_subscriber, 3);

// Subscribe to the wifi configuration channel
ZBUS_CHAN_ADD_OBS(config_wifi_channel, ble_thread_subscriber, 3);

// Subscribe to the wifi status channel
ZBUS_CHAN_ADD_OBS(status_wifi_channel, ble_thread_subscriber, 3);

// Subscribe to the Water pump status channel
ZBUS_CHAN_ADD_OBS(status_water_pump_channel, ble_thread_subscriber, 3);

// Subscribe to the Water valve status channel
ZBUS_CHAN_ADD_OBS(status_water_valve_channel, ble_thread_subscriber, 3);

// Subscribe to actuators water pump channel
ZBUS_CHAN_ADD_OBS(actuators_water_pump_channel, ble_thread_subscriber, 3);

// Subscribe to actuators water valve channel
ZBUS_CHAN_ADD_OBS(actuators_water_valve_channel, ble_thread_subscriber, 3);