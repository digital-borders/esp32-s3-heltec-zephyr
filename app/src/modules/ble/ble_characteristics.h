#define BLE_GATT_CPF_FORMAT_BOOLEAN 0x01
#define BLE_GATT_CPF_FORMAT_FLOAT32 0x14
#define BLE_GATT_CPF_FORMAT_UTF8S   0x19
#define BLE_GATT_CPF_FORMAT_BOOLEAN 0x01
#define BLE_GATT_CPF_FORMAT_UINT32  0x08

/// @brief Custom service Jocondo UUID
#define BT_UUID_CUSTOM_SERVICE_JOCONDO_UUID                                                        \
	BT_UUID_128_ENCODE(0x6930da7e, 0xfbcd, 0x4ebd, 0x9a25, 0x595ae83a3dbd)
#define BT_UUID_JOCONDO_SERVICE BT_UUID_DECLARE_128(BT_UUID_CUSTOM_SERVICE_JOCONDO_UUID)

/**
 *
 *
 * WIFI CHARACTERISTICS
 *
 *
 */

#define WIFI_STATUS_NAME "Wifi Status"
#define WIFI_SSID_NAME	 "Wifi SSID"
#define WIFI_PSK_NAME	 "Wifi PSK"

/// @brief Custom service WIFI UUID
#define BT_UUID_CUSTOM_SERVICE_WIFI_UUID                                                           \
	BT_UUID_128_ENCODE(0x6930da7e, 0xfbcd, 0x4ebd, 0x9a25, 0x595ae83a3dbe)
#define BT_UUID_WIFI_SERVICE BT_UUID_DECLARE_128(BT_UUID_CUSTOM_SERVICE_WIFI_UUID)

/// @brief Wifi SSID characteristic UUID
#define BT_UUID_WIFI_SSID_CHRC_UUID                                                                \
	BT_UUID_128_ENCODE(0x6fd63159, 0xb6e7, 0x4955, 0x9774, 0x13240f8d1d20)
#define BT_UUID_WIFI_SSID_CHRC BT_UUID_DECLARE_128(BT_UUID_WIFI_SSID_CHRC_UUID)

/// @brief Wifi PSK characteristic UUID
#define BT_UUID_WIFI_PSK_CHRC_UUID                                                                 \
	BT_UUID_128_ENCODE(0x6fd63159, 0xb6e7, 0x4955, 0x9774, 0x13240f8d1d21)
#define BT_UUID_WIFI_PSK_CHRC BT_UUID_DECLARE_128(BT_UUID_WIFI_PSK_CHRC_UUID)

/// @brief Wifi connection status characteristic UUID
#define BT_UUID_WIFI_STATUS_CHRC_UUID                                                              \
	BT_UUID_128_ENCODE(0x6fd63159, 0xb6e7, 0x4955, 0x9774, 0x13240f8d1d22)
#define BT_UUID_WIFI_STATUS_CHRC BT_UUID_DECLARE_128(BT_UUID_WIFI_STATUS_CHRC_UUID)

/**
 *
 *
 * WATER CHARACTERISTICS
 *
 *
 */

#define SENSOR_WATER_LEVEL_NAME		      "Water Level Sensor"
#define ACTUATOR_WATER_PUMP_RUNNING_TIME_NAME "Water Pump Status"
#define ACTUATOR_WATER_TANK_REFILL_NAME	      "Water Refill Status"

/// @brief Water level characteristic UUID
#define BT_UUID_WATER_LEVEL_CHRC_UUID                                                              \
	BT_UUID_128_ENCODE(0x156e0850, 0xcd46, 0x4d58, 0x9f20, 0xdd354f4b645b)
#define BT_UUID_WATER_LEVEL_CHRC BT_UUID_DECLARE_128(BT_UUID_WATER_LEVEL_CHRC_UUID)

/// @brief Actuator Water Pump status characteristic UUID
// Is the water pump running? how much time remaining?
#define BT_UUID_ACTUATOR_WATER_PUMP_RUNNING_TIME_CHRC_UUID                                         \
	BT_UUID_128_ENCODE(0x156e0850, 0xcd46, 0x4d58, 0x9f20, 0xdd354f4b645c)
#define BT_UUID_ACTUATOR_WATER_PUMP_RUNNING_TIME_CHRC                                              \
	BT_UUID_DECLARE_128(BT_UUID_ACTUATOR_WATER_PUMP_RUNNING_TIME_CHRC_UUID)

/// @brief Actuator Water Valve characteristic UUID
// Boolean
#define BT_UUID_ACTUATOR_WATER_TANK_REFILL_CHRC_UUID                                               \
	BT_UUID_128_ENCODE(0x156e0850, 0xcd46, 0x4d58, 0x9f20, 0xdd354f4b645d)
#define BT_UUID_ACTUATOR_WATER_TANK_REFILL_CHRC                                                    \
	BT_UUID_DECLARE_128(BT_UUID_ACTUATOR_WATER_TANK_REFILL_CHRC_UUID)

/**
 *
 *
 * CO
 *
 *
 */
#define SENSOR_CO_NAME "CO Sensor"

/// @brief Water level characteristic UUID
#define BT_UUID_CO_CHRC_UUID BT_UUID_128_ENCODE(0x156e0850, 0xcd46, 0x4d58, 0x9f20, 0xdd354f4b645e)
#define BT_UUID_CO_CHRC	     BT_UUID_DECLARE_128(BT_UUID_CO_CHRC_UUID)

/**
 *
 *
 *	USER MANAGEMENT CHARACTERISTICS
 *
 *
 */

#define CONFIG_USER_EMAIL_NAME "User Email"

/// @brief Water level characteristic UUID
#define BT_UUID_CONFIG_USER_EMAIL_CHRC_UUID                                                        \
	BT_UUID_128_ENCODE(0x3670141e, 0x5494, 0x4e58, 0xb174, 0xec14ae5ffd89)
#define BT_UUID_CONFIG_USER_EMAIL_CHRC BT_UUID_DECLARE_128(BT_UUID_CONFIG_USER_EMAIL_CHRC_UUID)

/**
 *
 *
 * FARMING CHARACTERISTICS
 *
 *
 */

#define CONFIG_FARMING_NAME "Farming Configuration"

/// @brief Farming characteristic UUID
#define BT_UUID_CONFIG_FARMING_CHRC_UUID                                                           \
	BT_UUID_128_ENCODE(0x300c11ca, 0x88f0, 0x4470, 0x9a9a, 0xbd7dab349667)
#define BT_UUID_CONFIG_FARMING_CHRC BT_UUID_DECLARE_128(BT_UUID_CONFIG_FARMING_CHRC_UUID)
