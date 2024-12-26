
#ifndef ZBUS_INFO_FIRMWARE_H
#define ZBUS_INFO_FIRMWARE_H
#include <stdint.h>
/// @brief Firmware information message structure
struct info_firmware_message {
	const struct {
		uint8_t major;
		uint8_t minor;
		uint8_t patch;
	} firmware_version;
};
#endif // ZBUS_INFO_FIRMWARE_H