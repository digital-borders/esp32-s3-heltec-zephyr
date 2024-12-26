#include <zephyr/zbus/zbus.h>

#include "device.h"
#include "firmware.h"
#include "hardware.h"

// Device information channel definition
ZBUS_CHAN_DEFINE(info_device_channel, struct info_device_message, NULL, NULL, ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.device_uuid = {0xdd, 0xdd}));
ZBUS_CHAN_DEFINE(info_firmware_channel, struct info_firmware_message, NULL, NULL,
		 ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.firmware_version = {.major = 1, .minor = 0, .patch = 0}));
ZBUS_CHAN_DEFINE(info_hardware_channel, struct info_hardware_message, NULL, NULL,
		 ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.hardware_version = {.major = 'A', .minor = "V01"}));