#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ENVIRONMENT_DEVICES_ds3231, LOG_LEVEL_ERR);

#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include "ds3231.h"

const struct device *get_ds3231_device(void)
{
	const struct device *const dev = DEVICE_DT_GET_ANY(adi_ds3231);

	if (dev == NULL) {
		/* No such node, or the node does not have status "okay". */
		LOG_ERR("No ds3231 device defined in the devicetree");
		return NULL;
	}

	if (!device_is_ready(dev)) {
		LOG_ERR("Device \"%s\" is not ready.", dev->name);
		return NULL;
	}

	LOG_INF("Found ds3231 compatible device with name \"%s\"", dev->name);

	return dev;
}
