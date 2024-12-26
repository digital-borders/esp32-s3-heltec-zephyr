#ifndef ZBUS_CONFIG_WIFI_H
#define ZBUS_CONFIG_WIFI_H

#include <stdint.h>

/// @brief WiFi configuration message structure
struct config_wifi_message {
    char ssid[32];
    char psk[64];
};

#endif // ZBUS_CONFIG_WIFI_H