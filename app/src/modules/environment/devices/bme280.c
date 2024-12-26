#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ENVIRONMENT_DEVICES_BME280, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include "bme280.h"

const struct device *get_bme280_device(void)
{
	const struct device *const dev = DEVICE_DT_GET_ANY(bosch_bme280);

	if (dev == NULL) {
		/* No such node, or the node does not have status "okay". */
		LOG_ERR("No BME280 device defined in the devicetree");
		return NULL;
	}

	if (!device_is_ready(dev)) {
        LOG_ERR("Device \"%s\" is not ready.",
                dev->name);
		return NULL;
	}

    LOG_INF("Found BME280 compatible device with name \"%s\"", dev->name);
    return dev;
}
