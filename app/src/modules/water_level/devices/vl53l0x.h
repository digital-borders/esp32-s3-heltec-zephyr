#ifndef VL53L0X_H
#define VL53L0X_H

#include <zephyr/device.h>

/// @brief Get a device structure from a devicetree node with compatible
/// "st,vl53l0x". (If there are multiple, just pick one.)
/// @return The BME280 device structure or NULL if not found or not ready.
const struct device *get_vl53l0x_device(void);

#endif // VL53L0X_H