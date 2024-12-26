#ifndef WIFI_ZBUS_H
#define WIFI_ZBUS_H
#include <zephyr/zbus/zbus.h>
// Channels to publish to
#include "zbus/status/wifi.h"
ZBUS_CHAN_DECLARE(status_wifi_channel);
// Channels to listen to
#include "zbus/config/wifi.h"
ZBUS_CHAN_DECLARE(config_wifi_channel);
// Subscriptions
ZBUS_SUBSCRIBER_DEFINE(wifi_thread_subscriber, 4);
ZBUS_CHAN_ADD_OBS(config_wifi_channel, wifi_thread_subscriber, 3);
#endif // WIFI_ZBUS_H