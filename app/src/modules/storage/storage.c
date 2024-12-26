#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(STORAGE, LOG_LEVEL_ERR);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>

#include "storage_const.h"
#include "storage_zbus.h"

static struct config_wifi_message wifi_message;

static struct config_user_message user_message;

static struct config_farming_str_message farming_str_message_nvs;
static struct config_farming_str_message farming_str_message_new;

static struct nvs_fs fs;

void checkWifiConfig()
{
	// Check if the wifi configuration is stored in STORAGE
	LOG_INF("Checking wifi configuration");
	// number of bytes read
	int rc = 0;
	rc = nvs_read(&fs, WIFI_SSID_STORAGE_INDEX, &wifi_message.ssid, WIFI_SSID_MAX_LENGTH);
	if (rc < 1) {
		LOG_ERR("Failed to read SSID from STORAGE");
		return;
	}

	LOG_INF("Read SSID from STORAGE: %s", wifi_message.ssid);
	// Read also the PSK
	rc = 0;
	rc = nvs_read(&fs, WIFI_PSK_STORAGE_INDEX, &wifi_message.psk, WIFI_PSK_MAX_LENGTH);
	if (rc < 1) {
		LOG_ERR("Failed to read PSK from STORAGE");
		return;
	}

	LOG_INF("Read PSK from STORAGE: %s", wifi_message.psk);
	LOG_DBG("Publishing wifi config from storage");
	// Pub messasge
	zbus_chan_pub(&config_wifi_channel, &wifi_message, K_FOREVER);
}

bool writeWifiConfig()
{
	int rc = 0;
	rc = nvs_write(&fs, WIFI_SSID_STORAGE_INDEX, &wifi_message.ssid, strlen(wifi_message.ssid));
	if (rc < 0) {
		LOG_ERR("Failed to write SSID to STORAGE");
		return false;
	}

	rc = nvs_write(&fs, WIFI_PSK_STORAGE_INDEX, &wifi_message.psk, strlen(wifi_message.psk));
	if (rc < 0) {
		LOG_ERR("Failed to write PSK to STORAGE");
		return false;
	}

	LOG_INF("Write SSID and PSK to STORAGE finished");
	return true;
}

void checkUserConfig()
{
	// Check if the wifi configuration is stored in STORAGE
	LOG_INF("Checking user configuration");
	// number of bytes read
	int rc = 0;
	rc = nvs_read(&fs, CONFIG_USER_EMAIL_STORAGE_INDEX, &user_message.email,
		      CONFIG_USER_EMAIL_MAX_LENGTH);
	if (rc < 1) {
		LOG_ERR("Failed to read User Email from STORAGE");
		return;
	}
	LOG_INF("Read User Email from STORAGE: %s", user_message.email);
	LOG_DBG("Publishing user config from storage");
	// Pub messasge
	zbus_chan_pub(&config_user_channel, &user_message, K_FOREVER);
}

void writeUserConfig()
{
	int rc = nvs_write(&fs, CONFIG_USER_EMAIL_STORAGE_INDEX, &user_message.email,
			   strlen(user_message.email));
	if (rc < 0) {
		LOG_ERR("Failed to write email to STORAGE");
	}
	LOG_INF("Write email to STORAGE finished - check bytes: %d - email bytes: %d", rc,
		strlen(user_message.email));
}

void checkFarmingConfig()
{
	// Check if the wifi configuration is stored in STORAGE
	LOG_INF("Checking farming configuration");
	// number of bytes read
	int rc = 0;

	// First read lenght
	uint32_t dataLenght = 0;
	rc = nvs_read(&fs, CONFIG_FARMING_STR_LENGHT_STORAGE_INDEX, &dataLenght,
		      sizeof(dataLenght));
	if (rc < 1) {
		LOG_ERR("Failed to read farming string length from STORAGE");
		return;
	}

	LOG_INF("Read farming string length from STORAGE: %d", dataLenght);

	rc = nvs_read(&fs, CONFIG_FARMING_STR_STORAGE_INDEX,
		      &farming_str_message_nvs.farming_json_string, CONFIG_FARMING_STR_MAX_LENGTH);
	if (rc < 1) {
		LOG_ERR("Failed to read farming string from STORAGE");
		return;
	}
	LOG_INF("Read farming string from STORAGE: %s - rc: %d",
		farming_str_message_nvs.farming_json_string, rc);
	LOG_DBG("Publishing farming config from storage");
	// Pub messasge
	zbus_chan_pub(&config_farming_str_channel, &farming_str_message_nvs, K_FOREVER);
}

void writeFarmingConfig()
{
	int rc = nvs_write(&fs, CONFIG_FARMING_STR_STORAGE_INDEX,
			   &farming_str_message_nvs.farming_json_string,
			   strlen(farming_str_message_nvs.farming_json_string));
	if (rc < 0) {
		LOG_ERR("Failed to write farming string to STORAGE");
		return;
	}
	LOG_INF("Write farming string to STORAGE finished - check bytes: %d - farming bytes: %d",
		rc, strlen(farming_str_message_nvs.farming_json_string));

	// write also length
	int dataLenght = strlen(farming_str_message_nvs.farming_json_string);
	rc = nvs_write(&fs, CONFIG_FARMING_STR_LENGHT_STORAGE_INDEX, &dataLenght,
		       sizeof(dataLenght));

	if (rc < 0) {
		LOG_ERR("Failed to write farming string length to STORAGE");
		return;
	}
	LOG_INF("Write farming string length to STORAGE finished - check bytes: %d - farming "
		"lenght "
		"bytes: %d",
		rc, sizeof(dataLenght));
}

/**
 * The water management thread
 */
int storage_thread(void)
{
	LOG_INF("Initializing nvs module");

	int rc = 0;
	struct flash_pages_info info;

	/* define the nvs file system by settings with:
	 *	sector_size equal to the pagesize,
	 *	3 sectors
	 *	starting at NVS_PARTITION_OFFSET
	 */
	fs.flash_device = NVS_PARTITION_DEVICE;
	if (!device_is_ready(fs.flash_device)) {
		LOG_ERR("Flash device %s is not ready\n", fs.flash_device->name);
		return 0;
	}
	fs.offset = NVS_PARTITION_OFFSET;
	rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
	if (rc) {
		LOG_ERR("Unable to get page info\n");
		return 0;
	}
	fs.sector_size = info.size;
	fs.sector_count = 3U;

	rc = nvs_mount(&fs);
	if (rc) {
		LOG_ERR("Flash Init failed\n");
		return 0;
	}

	LOG_INF("STORAGE module initialized");

	checkWifiConfig();

	checkUserConfig();

	checkFarmingConfig();

	while (1) {
		// wait for a message on read channel

		const struct zbus_channel *channel;

		zbus_sub_wait(&storage_thread_subscriber, &channel, K_FOREVER);

		LOG_INF("Received message on channel %s", zbus_chan_name(channel));

		// check the channel
		if (channel == &config_wifi_channel) {

			struct config_wifi_message message;

			zbus_chan_read(&config_wifi_channel, &message, K_FOREVER);

			// Check if something has changed
			if (strcmp(wifi_message.ssid, message.ssid) != 0 ||
			    strcmp(wifi_message.psk, message.psk) != 0) {
				// Update the wifi message
				memcpy(&wifi_message, &message, sizeof(struct config_wifi_message));
				LOG_INF("Wifi configuration updated");
				writeWifiConfig();
			}

		} else if (channel == &config_user_channel) {

			struct config_user_message message;

			zbus_chan_read(&config_user_channel, &message, K_FOREVER);

			// Check if something has changed
			if (strcmp(user_message.email, message.email) != 0) {
				// Update the user message
				memcpy(&user_message, &message, sizeof(struct config_user_message));
				LOG_INF("User configuration updated");
				writeUserConfig();
			}
		}

		else if (channel == &config_farming_str_channel) {


			zbus_chan_read(&config_farming_str_channel, &farming_str_message_new, K_FOREVER);

			// Check if something has changed
			if (strcmp(farming_str_message_nvs.farming_json_string,
				   farming_str_message_new.farming_json_string) != 0) {
				// Update the farming message
				memcpy(&farming_str_message_nvs, &farming_str_message_new,
				       sizeof(struct config_farming_str_message));
				LOG_INF("Farming configuration updated");
				writeFarmingConfig();
			}
		}
	}

	return 0;
}

// Define the storage threads
K_THREAD_DEFINE(storage_thread_id, 2048, storage_thread, NULL, NULL, NULL, 3, 0, 0);
