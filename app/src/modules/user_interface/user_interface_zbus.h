
#ifndef USER_INTERFACE_ZBUS_H
#define USER_INTERFACE_ZBUS_H
#include <zephyr/zbus/zbus.h>
// Channels to publish to

// Channels to listen to
#include "zbus/actuators/buzzer.h"
ZBUS_CHAN_DECLARE(actuators_buzzer_channel);

// Subscriptions
/**
 * Define an observer of type subscriber for the user interface thread
 */
ZBUS_SUBSCRIBER_DEFINE(user_interface_thread_subscriber, 4);

/**
 * Suscribe to the buzzer actuator channel
 */
ZBUS_CHAN_ADD_OBS(actuators_buzzer_channel, user_interface_thread_subscriber, 3);

#endif // USER_INTERFACE_ZBUS_H