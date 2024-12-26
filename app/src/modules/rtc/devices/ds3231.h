#ifndef DS3231_H
#define DS3231_H

#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>


/// @brief Get a device structure from a devicetree node with compatible
/// "maxim,ds3231". (If there are multiple, just pick one.)
/// @return The ds3231 device structure or NULL if not found or not ready.
const struct device *get_ds3231_device(void);

#endif // DS3231_H