#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(BLE, LOG_LEVEL_ERR);
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/services/bas.h>

#include <zephyr/zbus/zbus.h>

#include "utils.h"

#include "ble_zbus.h"
#include "ble_characteristics.h"

/// @brief The humidity sensor message
static struct sensors_humidity_message humidity_message = {
	.humidity = 0,
};

/// @brief The temperature sensor message
static struct sensors_temperature_message temperature_message = {
	.temperature = 0,
};

/// @brief The pressure sensor message
static struct sensors_pressure_message pressure_message = {
	.pressure = 0,
};

/// @brief The co sensor message
static struct sensors_co_message co_message = {
	.co = 0,
};

/// @brief The battery sensor message
static struct sensors_battery_message battery_message = {
	.percentage = 0,
};

/// @brief The water level sensor message
static struct sensors_water_level_message water_level_message = {
	.percentage = 0,
};

/// @brief The water pump running message
static struct actuators_water_pump_message water_pump_running_message = {.run = false,
									 .time_remaining = 0};

/// @brief The water pump running message
static struct actuators_water_valve_message water_valve_message = {.open = false};

/// @brief The wifi configuration message
static struct config_wifi_message wifi_message = {
	.ssid = "",
	.psk = "",
};

/// @brief The wifi status message
static struct status_wifi_message wifi_status_message = {.connected = false};

static struct config_user_message user_message = {
	.email = "",
};

static struct config_farming_str_message farming_str_message = {
	.farming_json_string = "",
};

static struct info_device_message device_info_message;

// BLE Subscription statuses
static uint8_t ble_humidity_subscribed = 0;
static uint8_t ble_temperature_subscribed = 0;
static uint8_t ble_pressure_subscribed = 0;
static uint8_t ble_co_subscribed = 0;
static int pressure = 1000;
static uint8_t ble_battery_subscribed = 0;
static uint8_t ble_water_level_subscribed = 0;
static uint8_t ble_wifi_status_subscribed = 0;
static uint8_t ble_water_pump_running_subscribed = 0;
static uint8_t ble_water_tank_refill_subscribed = 0;
static uint8_t ble_user_email_subscribed = 0;
static uint8_t ble_farming_subscribed = 0;

static uint8_t ble_water_valve_subscribed = 0;

/// @brief Advertising structure
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_ESS_VAL)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_JOCONDO_UUID),
};

/// @brief Function invoked when the ble client read the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @return
ssize_t read_temperature(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
			 uint16_t len, uint16_t offset)
{

	// we need to multiply the value by 100 to send it as an integer
	int temperature = (int)temperature_message.temperature * 100;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &temperature, sizeof(temperature));
}

static void temperature_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	LOG_INF("************ temperature_ccc_cfg_changed %u", value);

	ble_temperature_subscribed = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/// @brief Function invoked when the ble client read the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @return
ssize_t read_humidity(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
		      uint16_t len, uint16_t offset)
{

	int humidity = convert_humidity(humidity_message.humidity);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &humidity, sizeof(humidity));
}

static void humidity_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	LOG_INF("************ humidity_ccc_cfg_changed %u", value);

	ble_humidity_subscribed = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/// @brief Function invoked when the ble client read the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @return
ssize_t read_pressure(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
		      uint16_t len, uint16_t offset)
{

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &pressure, sizeof(pressure));
}

static void pressure_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	LOG_INF("************ pressure_ccc_cfg_changed %u", value);

	ble_pressure_subscribed = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/// @brief Function invoked when the ble client read the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @return
ssize_t read_water_level(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
			 uint16_t len, uint16_t offset)
{

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &water_level_message.percentage,
				 sizeof(water_level_message.percentage));
}

static void water_level_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	LOG_INF("************ water_level_ccc_cfg_changed %u", value);

	ble_water_level_subscribed = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/// @brief Function invoked when the ble client read the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @return
ssize_t read_co(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
		uint16_t offset)
{

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &co_message.co,
				 sizeof(co_message.co));
}

static void co_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	LOG_INF("************ co_ccc_cfg_changed %u", value);

	ble_co_subscribed = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/// @brief Function invoked when the ble client read the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @return
ssize_t read_water_pump_running(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
				uint16_t len, uint16_t offset)
{

	return bt_gatt_attr_read(conn, attr, buf, len, offset,
				 &water_pump_running_message.time_remaining,
				 sizeof(water_pump_running_message.time_remaining));
}

static void water_pump_running_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	LOG_INF("************ water_pump_running_ccc_cfg_changed %u", value);

	ble_water_pump_running_subscribed = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/// @brief Function invoked when the ble client write the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @param flags
/// @return
ssize_t write_water_pump_running(struct bt_conn *conn, const struct bt_gatt_attr *attr,
				 const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{

	memcpy(&water_pump_running_message.time_remaining, buf, len);
	// 4 true if time_remaining > 0
	water_pump_running_message.run = water_pump_running_message.time_remaining > 0;

	LOG_DBG("Publishing Water pump running for %d seconds from ble",
		water_pump_running_message.time_remaining);

	zbus_chan_pub(&actuators_water_pump_channel, &water_pump_running_message, K_FOREVER);

	return len;
}

static const struct bt_gatt_cpf water_pump_running_cpf = {
	.format = BLE_GATT_CPF_FORMAT_UINT32,
};

/// @brief Function invoked when the ble client read the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @return
ssize_t read_water_tank_refill(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
			       uint16_t len, uint16_t offset)
{

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &water_valve_message.open,
				 sizeof(water_valve_message.open));
}

static void water_tank_refill_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	LOG_INF("************ water_tank_refill_ccc_cfg_changed %u", value);

	ble_water_tank_refill_subscribed = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/// @brief Function invoked when the ble client write the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @param flags
/// @return
ssize_t write_water_tank_refill(struct bt_conn *conn, const struct bt_gatt_attr *attr,
				const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	// memcpy(water_valve_message.open, buf, len);
	// memcpy is not working with boolean
	water_valve_message.open = ((uint8_t)buf != 0);

	LOG_DBG("Publishing Water valve open: %d from ble", water_valve_message.open);

	zbus_chan_pub(&actuators_water_valve_channel, &water_valve_message, K_FOREVER);

	return len;
}

static const struct bt_gatt_cpf water_tank_refill_cpf = {
	.format = BLE_GATT_CPF_FORMAT_BOOLEAN,
};

/**
 *
 *  USER EMAIL
 *
 */

/// @brief Function invoked when the ble client read the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @return
ssize_t read_user_email(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
			uint16_t len, uint16_t offset)
{

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &user_message.email,
				 sizeof(user_message.email));
}

static void user_email_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	LOG_INF("************ user_email_ccc_cfg_changed %u", value);

	ble_user_email_subscribed = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/// @brief Function invoked when the ble client write the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @param flags
/// @return
ssize_t write_user_email(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags)
{
	memcpy(user_message.email, buf, len);

	LOG_DBG("Publishing user email: %s from ble", user_message.email);

	zbus_chan_pub(&config_user_channel, &user_message, K_FOREVER);

	return len;
}

static const struct bt_gatt_cpf user_email_cpf = {
	.format = BLE_GATT_CPF_FORMAT_UTF8S,
};

/**
 *
 *  FARMING
 *
 */

/// @brief Function invoked when the ble client read the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @return
ssize_t read_farming(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
		     uint16_t offset)
{

	return bt_gatt_attr_read(conn, attr, buf, len, offset,
				 &farming_str_message.farming_json_string,
				 sizeof(farming_str_message.farming_json_string));
}

static void farming_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	LOG_INF("************ farming_ccc_cfg_changed %u", value);

	ble_farming_subscribed = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/// @brief Function invoked when the ble client write the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @param flags
/// @return
ssize_t write_farming(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
		      uint16_t len, uint16_t offset, uint8_t flags)
{
	// Save to the farming_str_message
	memcpy(farming_str_message.farming_json_string, buf, len);

	LOG_DBG("Publishing farming: %s from ble", farming_str_message.farming_json_string);

	zbus_chan_pub(&config_farming_str_channel, &farming_str_message, K_FOREVER);

	return len;
}

static const struct bt_gatt_cpf farming_cpf = {
	.format = BLE_GATT_CPF_FORMAT_UTF8S,
};

/// @brief Function invoked when the ble client read the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @return
ssize_t read_ssid(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
		  uint16_t offset)
{

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &wifi_message.ssid,
				 sizeof(wifi_message.ssid));
}

/// @brief Function invoked when the ble client write the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @param flags
/// @return
ssize_t write_ssid(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
		   uint16_t len, uint16_t offset, uint8_t flags)
{
	memcpy(wifi_message.ssid, buf, len);

	LOG_DBG("Publishing WiFi ssid: %s from ble", wifi_message.ssid);

	zbus_chan_pub(&config_wifi_channel, &wifi_message, K_FOREVER);

	return len;
}

/// @brief Function invoked when the ble client write the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @param flags
/// @return
ssize_t write_psk(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
		  uint16_t len, uint16_t offset, uint8_t flags)
{
	memcpy(wifi_message.psk, buf, len);

	LOG_DBG("Publishing WiFi psk: %s from ble", wifi_message.psk);

	zbus_chan_pub(&config_wifi_channel, &wifi_message, K_FOREVER);

	return len;
}
/// @brief Function invoked when the ble client read the characteristic
/// @param conn
/// @param attr
/// @param buf
/// @param len
/// @param offset
/// @return
ssize_t read_wifi_status(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
			 uint16_t len, uint16_t offset)
{

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &wifi_status_message.connected,
				 sizeof(wifi_status_message.connected));
}

static void wifi_status_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	LOG_INF("************ wifi_status_ccc_cfg_changed %u", value);

	ble_wifi_status_subscribed = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static const struct bt_gatt_cpf co_cpf = {
	.format = BLE_GATT_CPF_FORMAT_FLOAT32,
};

static const struct bt_gatt_cpf water_level_cpf = {
	.format = BLE_GATT_CPF_FORMAT_FLOAT32,
};
static const struct bt_gatt_cpf wifi_status_cpf = {
	.format = BLE_GATT_CPF_FORMAT_BOOLEAN,
};
static const struct bt_gatt_cpf ssid_cpf = {
	.format = BLE_GATT_CPF_FORMAT_UTF8S,
};
static const struct bt_gatt_cpf psk_cpf = {
	.format = BLE_GATT_CPF_FORMAT_UTF8S,
};
/* Vendor Primary Service Declaration */
// More info on index on
// https://devzone.nordicsemi.com/f/nordic-q-a/83538/gatt-service-bt_gatt_notify-argument-const-structbt_gatt_attr-attr/347084
BT_GATT_SERVICE_DEFINE(
	jocondo_service, BT_GATT_PRIMARY_SERVICE(BT_UUID_JOCONDO_SERVICE),

	// BT_GATT_CHARACTERISTIC vale "due" index quando si usa la funzione bt_gatt_notify
	BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, read_temperature, NULL,
			       &temperature_message.temperature),
	// quindi l'index di questo attributo è 3
	BT_GATT_CCC(temperature_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),

	// index 4 e 5
	BT_GATT_CHARACTERISTIC(BT_UUID_PRESSURE, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, read_pressure, NULL, &pressure),
	// index 6
	BT_GATT_CCC(pressure_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),

	// index 7 e 8
	BT_GATT_CHARACTERISTIC(BT_UUID_HUMIDITY, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, read_humidity, NULL, &humidity_message.humidity),
	// index 9
	BT_GATT_CCC(humidity_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),

	// index 10 e 11
	BT_GATT_CHARACTERISTIC(BT_UUID_WATER_LEVEL_CHRC, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, read_water_level, NULL,
			       &water_level_message.percentage),
	// index 12
	BT_GATT_CCC(water_level_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),
	// index 13
	BT_GATT_CPF(&water_level_cpf),
	// index 14
	BT_GATT_CUD(SENSOR_WATER_LEVEL_NAME, BT_GATT_PERM_READ),

	// index 15 e 16
	BT_GATT_CHARACTERISTIC(BT_UUID_ACTUATOR_WATER_PUMP_RUNNING_TIME_CHRC,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, read_water_pump_running,
			       write_water_pump_running,
			       &water_pump_running_message.time_remaining),
	// index 17
	BT_GATT_CCC(water_pump_running_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),
	// index 18
	BT_GATT_CPF(&water_pump_running_cpf),
	// index 19
	BT_GATT_CUD(ACTUATOR_WATER_PUMP_RUNNING_TIME_NAME, BT_GATT_PERM_READ),

	// index 20 e 21
	BT_GATT_CHARACTERISTIC(BT_UUID_ACTUATOR_WATER_TANK_REFILL_CHRC,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, read_water_tank_refill,
			       write_water_tank_refill, &water_valve_message.open),
	// index 22
	BT_GATT_CCC(water_tank_refill_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),
	// index 23
	BT_GATT_CPF(&water_tank_refill_cpf),
	// index 24
	BT_GATT_CUD(ACTUATOR_WATER_TANK_REFILL_NAME, BT_GATT_PERM_READ),

	// index 25 e 26
	BT_GATT_CHARACTERISTIC(BT_UUID_CONFIG_USER_EMAIL_CHRC,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT, read_user_email,
			       write_user_email, &user_message.email),
	// index 27
	BT_GATT_CCC(user_email_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),
	// index 28
	BT_GATT_CPF(&user_email_cpf),
	// index 29
	BT_GATT_CUD(CONFIG_USER_EMAIL_NAME, BT_GATT_PERM_READ),

	// index 30 e 31
	BT_GATT_CHARACTERISTIC(BT_UUID_CONFIG_FARMING_CHRC, BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT, read_farming,
			       write_farming, &farming_str_message.farming_json_string),
	// index 32
	BT_GATT_CCC(farming_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),
	// index 33
	BT_GATT_CPF(&farming_cpf),
	// index 34
	BT_GATT_CUD(CONFIG_FARMING_NAME, BT_GATT_PERM_READ),

	// index 35 e 36
	BT_GATT_CHARACTERISTIC(BT_UUID_CO_CHRC, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, read_co, NULL, &co_message.co),
	// index 37
	BT_GATT_CCC(co_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),
	// index 38
	BT_GATT_CPF(&co_cpf),
	// index 39
	BT_GATT_CUD(SENSOR_CO_NAME, BT_GATT_PERM_READ),

);

BT_GATT_SERVICE_DEFINE(
	wifi_service, BT_GATT_PRIMARY_SERVICE(BT_UUID_WIFI_SERVICE),

	BT_GATT_CHARACTERISTIC(BT_UUID_WIFI_STATUS_CHRC, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, read_wifi_status, NULL,
			       &wifi_status_message.connected),
	BT_GATT_CCC(wifi_status_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT),
	BT_GATT_CPF(&wifi_status_cpf), BT_GATT_CUD(WIFI_STATUS_NAME, BT_GATT_PERM_READ),

	BT_GATT_CHARACTERISTIC(BT_UUID_WIFI_SSID_CHRC, BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, read_ssid, write_ssid,
			       &wifi_message.ssid),
	BT_GATT_CPF(&ssid_cpf), BT_GATT_CUD(WIFI_SSID_NAME, BT_GATT_PERM_READ),

	BT_GATT_CHARACTERISTIC(BT_UUID_WIFI_PSK_CHRC, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE, NULL,
			       write_psk, &wifi_message.psk),
	BT_GATT_CPF(&psk_cpf), BT_GATT_CUD(WIFI_PSK_NAME, BT_GATT_PERM_READ),

);

void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
	LOG_INF("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static struct bt_gatt_cb gatt_callbacks = {.att_mtu_updated = mtu_updated};
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Passkey for %s: %06u\n", addr, passkey);
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
	.passkey_display = auth_passkey_display,
	.passkey_entry = NULL,
	.cancel = auth_cancel,
};

void setDeviceNameAndStartAdv()
{
	// Disable advertising
	int err = bt_le_adv_stop();

	LOG_DBG("Advertising stopped (err %d)\n", err);

	// fill the device name
	char dev_name[32];
	int dev_name_len = 0;
	dev_name_len = sprintf(dev_name, "jocondo-%02x%02x%02x%02x%02x%02x",
			       device_info_message.device_uuid[0], device_info_message.device_uuid[1],
			       device_info_message.device_uuid[2], device_info_message.device_uuid[3],
			       device_info_message.device_uuid[4], device_info_message.device_uuid[5]);

	LOG_INF("Setting device name: %s", dev_name);
	/// @brief Name advertising structure
	const struct bt_data sd[] = {
		BT_DATA(BT_DATA_NAME_COMPLETE, dev_name, dev_name_len),
	};

	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

	if (err) {
		LOG_INF("Advertising failed to start (err %d)\n", err);
		return;
	}

	LOG_INF("Advertising successfully started\n");
}

/// @brief Wait for new zbus messages
/// @return True if the configuration was updated, false otherwise
static void checkForUpdate()
{
	const struct zbus_channel *channel;

	zbus_sub_wait(&ble_thread_subscriber, &channel, K_FOREVER);

	LOG_DBG("Received message on channel %s", zbus_chan_name(channel));

	// Check wich channel has been updated
	if (channel == &info_device_channel) {

		zbus_chan_read(&info_device_channel, &device_info_message, K_FOREVER);
		setDeviceNameAndStartAdv();
	} else if (channel == &config_farming_str_channel) {
		zbus_chan_read(&config_farming_str_channel, &farming_str_message, K_FOREVER);

		LOG_DBG("Changed farming config, update ble characteristic");

		if (ble_farming_subscribed) {

			bt_gatt_notify(NULL, &jocondo_service.attrs[30],
				       &farming_str_message.farming_json_string,
				       sizeof(farming_str_message.farming_json_string));
		}

	} else if (channel == &config_user_channel) {
		zbus_chan_read(&config_user_channel, &user_message, K_FOREVER);

		LOG_DBG("User email: %s", user_message.email);

		if (ble_user_email_subscribed) {

			bt_gatt_notify(NULL, &jocondo_service.attrs[25], &user_message.email,
				       sizeof(user_message.email));
		}

	} else if (channel == &status_wifi_channel) {
		zbus_chan_read(&status_wifi_channel, &wifi_status_message, K_FOREVER);

		LOG_DBG("Wifi status: %d", wifi_status_message.connected);

		if (ble_wifi_status_subscribed) {

			bt_gatt_notify(NULL, &wifi_service.attrs[1], &wifi_status_message.connected,
				       sizeof(wifi_status_message.connected));
		}

	} else if (channel == &sensors_humidity_channel) {

		zbus_chan_read(&sensors_humidity_channel, &humidity_message, K_FOREVER);

		LOG_DBG("Humidity: %d", (int)humidity_message.humidity);

		if (ble_humidity_subscribed) {

			int humidity = convert_humidity(humidity_message.humidity);

			bt_gatt_notify(NULL, &jocondo_service.attrs[7], &humidity,
				       sizeof(humidity));
		}

	} else if (channel == &sensors_pressure_channel) {

		zbus_chan_read(&sensors_pressure_channel, &pressure_message, K_FOREVER);

		LOG_DBG("pressure: %d", (int)pressure_message.pressure);

		if (ble_pressure_subscribed) {

			bt_gatt_notify(NULL, &jocondo_service.attrs[4], &pressure_message.pressure,
				       sizeof(pressure_message.pressure));
		}

	} else if (channel == &sensors_co_channel) {

		zbus_chan_read(&sensors_co_channel, &co_message, K_FOREVER);

		LOG_DBG("co: %d", (int)co_message.co);

		if (ble_co_subscribed) {

			bt_gatt_notify(NULL, &jocondo_service.attrs[35], &co_message.co,
				       sizeof(co_message.co));
		}

	} else if (channel == &sensors_temperature_channel) {

		zbus_chan_read(&sensors_temperature_channel, &temperature_message, K_FOREVER);

		LOG_DBG("Temperature: %d", (int)temperature_message.temperature);

		if (ble_temperature_subscribed) {

			// we need to multiply the value by 100 to send it as an integer
			int temperature = (int)temperature_message.temperature * 100;

			bt_gatt_notify(NULL, &jocondo_service.attrs[1], &temperature,
				       sizeof(temperature));
		}
	} else if (channel == &sensors_water_level_channel) {

		zbus_chan_read(&sensors_water_level_channel, &water_level_message, K_FOREVER);

		LOG_DBG("Water level: %d", water_level_message.percentage);
		if (ble_water_level_subscribed) {

			bt_gatt_notify(NULL, &jocondo_service.attrs[10],
				       &water_level_message.percentage,
				       sizeof(water_level_message.percentage));
		}
	} else if (channel == &actuators_water_pump_channel) {

		zbus_chan_read(&actuators_water_pump_channel, &water_pump_running_message,
			       K_FOREVER);
		LOG_INF("Received water pump message with run: %d and time remaining: %d",
			water_pump_running_message.run, water_pump_running_message.time_remaining);

		if (ble_water_pump_running_subscribed) {

			bt_gatt_notify(NULL, &jocondo_service.attrs[15],
				       &water_pump_running_message.time_remaining,
				       sizeof(water_pump_running_message.time_remaining));
		}

	} else if (channel == &actuators_water_valve_channel) {

		zbus_chan_read(&actuators_water_valve_channel, &water_valve_message, K_FOREVER);
		LOG_INF("Received water valve message with open: %d", water_valve_message.open);

		if (ble_water_valve_subscribed) {

			bt_gatt_notify(NULL, &jocondo_service.attrs[20], &water_valve_message.open,
				       sizeof(water_valve_message.open));
		}

	} else if (channel == &sensors_battery_channel) {
		zbus_chan_read(&sensors_battery_channel, &battery_message, K_FOREVER);
		bt_bas_set_battery_level((int)battery_message.percentage);
	}

	else {
		LOG_INF("Received message on channel %s", zbus_chan_name(channel));
	}
}

/// @brief The ble management thread
/// @param  None
/// @return The thread return code
int ble_management_thread(void)
{
	int err;

	LOG_INF("Initializing ble module");

	/* Sleep 1 seconds to allow initialization of wifi driver. */
	k_sleep(K_SECONDS(1));

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return 0;
	}
	LOG_DBG("Bluetooth initialized");

	// Load the settings
	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
		LOG_INF("Bluetooth settings loaded\n");
	}

	bt_gatt_cb_register(&gatt_callbacks);

	bt_conn_auth_cb_register(&auth_cb_display);

	LOG_DBG("Bluetooth ready, wait for device UUID");

	// wifi simple connection manager
	while (1) {
		// wait for a message on some channel
		checkForUpdate();
	}
}

// Define the ble management thread
K_THREAD_DEFINE(ble_management_thread_id, 4096, ble_management_thread, NULL, NULL, NULL, 3, 0, 0);