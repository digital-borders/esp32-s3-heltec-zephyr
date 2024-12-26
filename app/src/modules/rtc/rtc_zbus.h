
#ifndef RTC_ZBUS_H
#define RTC_ZBUS_H
#include <zephyr/zbus/zbus.h>
// Channels to publish to
#include "zbus/sensors/rtc.h"
ZBUS_CHAN_DECLARE(sensors_rtc_channel);

// Channels to listen to
#include "zbus/status/wifi.h"
ZBUS_CHAN_DECLARE(status_wifi_channel);

// Subscriptions
ZBUS_SUBSCRIBER_DEFINE(rtc_thread_subscriber, 1);
ZBUS_CHAN_ADD_OBS(status_wifi_channel, rtc_thread_subscriber, 4);

#endif // RTC_ZBUS_H