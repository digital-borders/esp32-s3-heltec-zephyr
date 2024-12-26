#ifndef ZBUS_SENSORS_RTC_H
#define ZBUS_SENSORS_RTC_H

#include <stdint.h>
#include <zephyr/zbus/zbus.h>

/// @brief RTC sensor message structure
struct sensors_rtc_message {
	// Current time
	uint32_t time;
};

#endif // ZBUS_SENSORS_RTC_H