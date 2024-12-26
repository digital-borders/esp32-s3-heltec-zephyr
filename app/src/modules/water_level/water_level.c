#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(WATER_LEVEL, LOG_LEVEL_ERR);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

#include "./devices/vl53l0x.h"

#include "water_level_zbus.h"

// The VL53L0X device
static const struct device *vl53l0x;

int water_level_thread(void)
{
	LOG_INF("Initializing water level module");

	vl53l0x = get_vl53l0x_device();

	// check if the device is valid
	if (vl53l0x == NULL) {
		LOG_ERR("Could not initialize VL53L0X sensor.");
		return -1;
	}

	while (1) {
		// read the water level
		struct sensor_value sensor_water_level;
		int ret;

		ret = sensor_sample_fetch(vl53l0x);

		ret = sensor_channel_get(vl53l0x, SENSOR_CHAN_DISTANCE, &sensor_water_level);
		float water_level = sensor_value_to_float(&sensor_water_level);
		
		// We now that the sensor will output a row value of 0.100 when the water is full
		// we also now that the sensor will output a raw value of 0.800 when the water is
		// empty we can use this information to calculate the percentage of the water level

		float fullValue = 0.083;
		float emptyValue = 0.800;
		int percentage = (int)((water_level - emptyValue) / (fullValue - emptyValue) * 100);

		struct sensors_water_level_message message = {.percentage = percentage};

		LOG_INF("Water level: %.3f - %d", water_level, percentage);

		zbus_chan_pub(&sensors_water_level_channel, &message, K_FOREVER);

		// wait for the next iteration
		k_sleep(K_MSEC(CONFIG_SENSORS_WATER_LEVEL_FREQUENCY));
	}

	return 0;
}

// Define the sensor threads
K_THREAD_DEFINE(water_level_thread_id, 1024, water_level_thread, NULL, NULL, NULL, 3, 0, 0);
