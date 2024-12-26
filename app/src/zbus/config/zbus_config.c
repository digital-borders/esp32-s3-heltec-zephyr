#include <zephyr/zbus/zbus.h>

#include "wifi.h"
#include "sensors.h"
#include "mode.h"
#include "user.h"
#include "farming.h"

// User configuration channel
ZBUS_CHAN_DEFINE(config_user_channel, struct config_user_message, NULL, NULL, ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.email = ""));

// Wifi configuration channel
ZBUS_CHAN_DEFINE(config_wifi_channel, struct config_wifi_message, NULL, NULL, ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.ssid = "", .psk = ""));

// Sensors configuration channel
ZBUS_CHAN_DEFINE(config_sensors_channel, struct config_sensors_message, NULL, NULL,
		 ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.temperature_poll_interval_ms = 1000,
			       .humidity_poll_interval_ms = 2000, .pressure_poll_interval_ms = 3000,
			       .water_level_poll_interval_ms = 5000,
			       .battery_poll_interval_ms = 60000));

// Operational mode configuration channel
ZBUS_CHAN_DEFINE(config_mode_channel, struct config_mode_message, NULL, NULL, ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.mode = ZBUS_CONFIG_MODE_BASE_STATION, ));

// Wifi configuration channel
ZBUS_CHAN_DEFINE(config_farming_str_channel, struct config_farming_str_message, NULL, NULL,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.farming_json_string = "", ));