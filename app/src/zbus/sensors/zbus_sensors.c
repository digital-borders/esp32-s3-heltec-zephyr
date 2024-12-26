#include <zephyr/zbus/zbus.h>

#include "humidity.h"
#include "temperature.h"
#include "pressure.h"
#include "water_level.h"
#include "battery.h"
#include "co.h"
#include "rtc.h"

// Humidity sensor channel definition
ZBUS_CHAN_DEFINE(sensors_humidity_channel, struct sensors_humidity_message, NULL, NULL,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.humidity = 0.0));

// Temperature sensor channel definition
ZBUS_CHAN_DEFINE(sensors_temperature_channel, struct sensors_temperature_message, NULL, NULL,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.temperature = 0.0));

// Pressure sensor channel definition
ZBUS_CHAN_DEFINE(sensors_pressure_channel, struct sensors_pressure_message, NULL, NULL,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.pressure = 0));

// Water level sensor channel definition
ZBUS_CHAN_DEFINE(sensors_water_level_channel, struct sensors_water_level_message, NULL, NULL,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.percentage = 0));

// Battery sensor channel definition
ZBUS_CHAN_DEFINE(sensors_battery_channel, struct sensors_battery_message, NULL, NULL,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.percentage = 0));

// CO sensor channel definition
ZBUS_CHAN_DEFINE(sensors_co_channel, struct sensors_co_message, NULL, NULL, ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.co = 0));

// RTC sensor channel definition
ZBUS_CHAN_DEFINE(sensors_rtc_channel, struct sensors_rtc_message, NULL, NULL, ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.time = 0));