
#ifndef ZBUS_INFO_HARDWARE_H
#define ZBUS_INFO_HARDWARE_H
#include <stdint.h>
/// @brief Hardware information message structure
struct info_hardware_message {
	const struct {
		char major;
		char minor[4];
	} hardware_version;
};
#endif // ZBUS_INFO_HARDWARE_H