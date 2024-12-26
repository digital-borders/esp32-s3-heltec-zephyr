#ifndef ZBUS_STATUS_BLE_H
#define ZBUS_STATUS_BLE_H

#include <stdint.h>
#include <stdbool.h>

/// @brief Ble status message structure
struct status_ble_message {
    bool connected;
};

#endif // ZBUS_STATUS_BLE_H