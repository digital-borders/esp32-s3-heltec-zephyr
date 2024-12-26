#ifndef MQTT_ZBUS_H
#define MQTT_ZBUS_H

#include <zephyr/zbus/zbus.h>

// Channels to publish to
#include "zbus/status/mqtt.h"
ZBUS_CHAN_DECLARE(status_mqtt_channel);

#include "zbus/config/farming.h"
ZBUS_CHAN_DECLARE(config_farming_str_channel);

// Channels to listen to
#include "zbus/status/wifi.h"
ZBUS_CHAN_DECLARE(status_wifi_channel);

#include "zbus/info/device.h"
ZBUS_CHAN_DECLARE(info_device_channel);

#include "zbus/sensors/temperature.h"
ZBUS_CHAN_DECLARE(sensors_temperature_channel);

#include "zbus/sensors/humidity.h"
ZBUS_CHAN_DECLARE(sensors_humidity_channel);

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

// Subscriptions
ZBUS_SUBSCRIBER_DEFINE(mqtt_zbus_consumer_thread_subscriber, 12);
ZBUS_CHAN_ADD_OBS(sensors_temperature_channel, mqtt_zbus_consumer_thread_subscriber, 3);
ZBUS_CHAN_ADD_OBS(sensors_humidity_channel, mqtt_zbus_consumer_thread_subscriber, 3);
ZBUS_CHAN_ADD_OBS(sensors_pressure_channel, mqtt_zbus_consumer_thread_subscriber, 3);
ZBUS_CHAN_ADD_OBS(sensors_co_channel, mqtt_zbus_consumer_thread_subscriber, 3);
ZBUS_CHAN_ADD_OBS(sensors_battery_channel, mqtt_zbus_consumer_thread_subscriber, 3);
ZBUS_CHAN_ADD_OBS(sensors_water_level_channel, mqtt_zbus_consumer_thread_subscriber, 3);
ZBUS_CHAN_ADD_OBS(actuators_water_pump_channel, mqtt_zbus_consumer_thread_subscriber, 3);
ZBUS_CHAN_ADD_OBS(actuators_water_valve_channel, mqtt_zbus_consumer_thread_subscriber, 3);
ZBUS_CHAN_ADD_OBS(status_wifi_channel, mqtt_zbus_consumer_thread_subscriber, 3);
ZBUS_CHAN_ADD_OBS(info_device_channel, mqtt_zbus_consumer_thread_subscriber, 3);

#endif // MQTT_ZBUS_H