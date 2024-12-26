#ifndef ZBUS_CONFIG_BLE_H
#define ZBUS_CONFIG_BLE_H

#include <stdint.h>

/// @brief Ble configuration message structure
struct config_ble_message {
    char ssid[32];
    char psk[64];
};

#endif // ZBUS_CONFIG_BLE_H