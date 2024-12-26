#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(WATER_LEVEL_DEVICES_BME280, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include "vl53l0x.h"


#define SENSOR_NODE DT_NODELABEL(vl53l0x)

const struct device *get_vl53l0x_device(void)
{
	// const struct device *const dev = DEVICE_DT_GET_ANY(st_vl53l0x);
    static const struct device *const dev = DEVICE_DT_GET(SENSOR_NODE);

	if (dev == NULL) {
		/* No such node, or the node does not have status "okay". */
		LOG_ERR("No VL53L0X device defined in the devicetree");
		return NULL;
	}

	if (!device_is_ready(dev)) {
        LOG_ERR("Device \"%s\" is not ready.",
                dev->name);
		return NULL;
	}

    LOG_INF("Found VL53L0X compatible device with name \"%s\"", dev->name);
	return dev;
}