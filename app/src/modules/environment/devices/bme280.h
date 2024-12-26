#ifndef BME280_H
#define BME280_H

#include <zephyr/device.h>

/// @brief Get a device structure from a devicetree node with compatible
/// "bosch,bme280". (If there are multiple, just pick one.)
/// @return The BME280 device structure or NULL if not found or not ready.
const struct device *get_bme280_device(void);

#endif // BME280_H