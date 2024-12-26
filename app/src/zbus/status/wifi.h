#ifndef ZBUS_STATUS_WIFI_H
#define ZBUS_STATUS_WIFI_H

#include <stdint.h>
#include <stdbool.h>

/// @brief Wifi status message structure
struct status_wifi_message {
    bool connected;
};

#endif // ZBUS_STATUS_WIFI_H