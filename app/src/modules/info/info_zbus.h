#ifndef INFO_ZBUS_H
#define INFO_ZBUS_H
#include <zephyr/zbus/zbus.h>
// Channels to publish to
#include "zbus/info/device.h"
ZBUS_CHAN_DECLARE(info_device_channel);
// Channels to listen to
// none
// Subscriptions
// none
#endif // INFO_ZBUS_H