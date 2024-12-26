#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(BATTERY, LOG_LEVEL_ERR);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

#include "battery_zbus.h"

int battery_thread(void)
{
	LOG_INF("Initializing battery module");

	int battery_level = 100;

	while (1) {
		// update the battery level
		struct sensors_battery_message message = {.percentage = battery_level};

		// publish the data
		LOG_DBG("Publishing battery level = %.1f.", (double)message.percentage);

		zbus_chan_pub(&sensors_battery_channel, &message, K_FOREVER);

		// wait for the next iteration
		k_sleep(K_SECONDS(60));
	}

	return 0;
}

// Define the sensor threads
// K_THREAD_DEFINE(battery_thread_id, 2048, battery_thread, NULL, NULL, NULL, 3, 0, 0);
