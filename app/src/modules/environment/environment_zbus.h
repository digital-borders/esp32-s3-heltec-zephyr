
#ifndef ENVIRONMENT_ZBUS_H
#define ENVIRONMENT_ZBUS_H
#include <zephyr/zbus/zbus.h>
// Channels to publish to
#include "zbus/sensors/humidity.h"
ZBUS_CHAN_DECLARE(sensors_humidity_channel);
#include "zbus/sensors/temperature.h"
ZBUS_CHAN_DECLARE(sensors_temperature_channel);
#include "zbus/sensors/pressure.h"
ZBUS_CHAN_DECLARE(sensors_pressure_channel);
#include "zbus/sensors/co.h"
ZBUS_CHAN_DECLARE(sensors_co_channel);
// Channels to listen to
// none
// Subscriptions
// none
#endif // ENVIRONMENT_ZBUS_H