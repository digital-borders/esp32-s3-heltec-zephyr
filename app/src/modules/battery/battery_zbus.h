
#ifndef BATTERY_ZBUS_H
#define BATTERY_ZBUS_H
#include <zephyr/zbus/zbus.h>
// Channels to publish to
#include "zbus/sensors/battery.h"
ZBUS_CHAN_DECLARE(sensors_battery_channel);
// Channels to listen to

// Subscriptions

#endif // BATTERY_ZBUS_H