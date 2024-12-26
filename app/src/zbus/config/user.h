#ifndef ZBUS_CONFIG_USER_H
#define ZBUS_CONFIG_USER_H

#include <stdint.h>

/// @brief User configuration message structure
struct config_user_message {
    char email[32];
};

#endif // ZBUS_CONFIG_USER_H