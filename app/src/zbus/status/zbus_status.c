#include <zephyr/zbus/zbus.h>

#include "ble.h"
#include "wifi.h"
#include "water_pump.h"
#include "water_valve.h"
#include "mqtt.h"

// BLE status channel definition
ZBUS_CHAN_DEFINE(
    status_ble_channel,
    struct status_ble_message,
    NULL,
    NULL,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT(
        .connected = false,
    ));

// Wifi status channel definition
ZBUS_CHAN_DEFINE(
    status_wifi_channel,
    struct status_wifi_message,
    NULL,
    NULL,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT(
        .connected = false,
    ));

// Mqtt status channel definition
ZBUS_CHAN_DEFINE(
    status_mqtt_channel,
    struct status_mqtt_message,
    NULL,
    NULL,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT(
        .connected = false,
    ));

// Water pump status channel
ZBUS_CHAN_DEFINE(
    status_water_pump_channel,
    struct status_water_pump_message,
    NULL,
    NULL,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT(
        .running = false
    ));

// Water valve status channel
ZBUS_CHAN_DEFINE(
    status_water_valve_channel,
    struct status_water_valve_message,
    NULL,
    NULL,
    ZBUS_OBSERVERS_EMPTY,
    ZBUS_MSG_INIT(
        .opened = false
    ));