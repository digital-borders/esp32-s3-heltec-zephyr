
#ifndef STORAGE_ZBUS_H
#define STORAGE_ZBUS_H
#include <zephyr/zbus/zbus.h>

// Channels to listen and publish to
#include "zbus/config/wifi.h"
ZBUS_CHAN_DECLARE(config_wifi_channel);

#include "zbus/config/user.h"
ZBUS_CHAN_DECLARE(config_user_channel);

#include "zbus/config/farming.h"
ZBUS_CHAN_DECLARE(config_farming_str_channel);

// Subscriptions
// Define an observer of type subscriber for the storage module
ZBUS_SUBSCRIBER_DEFINE(storage_thread_subscriber, 4);

// Subscribe to wifi config channel
ZBUS_CHAN_ADD_OBS(config_wifi_channel, storage_thread_subscriber, 3);

// Subscribe to user config channel
ZBUS_CHAN_ADD_OBS(config_user_channel, storage_thread_subscriber, 3);

// Subscribe to farming config channel
ZBUS_CHAN_ADD_OBS(config_farming_str_channel, storage_thread_subscriber, 3);

#endif // STORAGE_ZBUS_H