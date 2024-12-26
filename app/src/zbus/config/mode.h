#ifndef ZBUS_CONFIG_MODE_H
#define ZBUS_CONFIG_MODE_H

#include <stdint.h>

/// @brief Operational mode configuration message structure
struct config_mode_message {
    enum {
        ZBUS_CONFIG_MODE_BASE_STATION,
        ZBUS_CONFIG_MODE_NODE
    } mode;
};

#endif // ZBUS_CONFIG_MODE_H