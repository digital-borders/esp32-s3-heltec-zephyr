#ifndef ZBUS_INFO_DEVICE_H
#define ZBUS_INFO_DEVICE_H
#include <stdint.h>
/// @brief Device information message structure
struct info_device_message {
	uint8_t device_uuid[6];
};
#endif // ZBUS_INFO_DEVICE_H