#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(RTC, LOG_LEVEL_ERR);

#include <zephyr/net/sntp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <zephyr/sys/timeutil.h>

#include "./devices/ds3231.h"

#include "rtc_zbus.h"

static struct sensors_rtc_message rtc_message = {
	.time = 0,
};

#define SNTP_PORT 123

#define SERVER_ADDR "193.204.114.232"

// The DS3231 device
static const struct device *ds3231;

static struct rtc_time set_t, get_t;

void readTimeAndPublish()
{
	int ret = rtc_get_time(ds3231, &get_t);
	if (ret != 0) {
		LOG_ERR("Error getting time!!");
	} else {
		LOG_DBG("Date/time is %d-%d-%d   %d:%d:%d\n", get_t.tm_year + 1900,
			get_t.tm_mon + 1, get_t.tm_mday, get_t.tm_hour, get_t.tm_min, get_t.tm_sec);

		// Publish the time
		rtc_message.time = mktime(&get_t);
		zbus_chan_pub(&sensors_rtc_channel, &rtc_message, K_FOREVER);
	}
}

bool setRtcTimeFromSntp()
{
	int ret;

	struct sntp_ctx ctx;
	struct sockaddr_in addr;
	struct sntp_time sntp_time;
	int rv;
	/* ipv4 */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SNTP_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &addr.sin_addr);

	rv = sntp_init(&ctx, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (rv < 0) {
		LOG_ERR("Failed to init SNTP IPv4 ctx: %d", rv);
		return false;
	}

	LOG_INF("SNTP IPv4 ctx initialized rv: %d", rv);

	LOG_INF("Sending SNTP IPv4 request...");
	rv = sntp_query(&ctx, 4 * MSEC_PER_SEC, &sntp_time);
	if (rv < 0) {
		LOG_ERR("SNTP IPv4 request failed: %d", rv);
		return false;
	}

	LOG_INF("status: %d", rv);
	LOG_INF("time since Epoch: high word: %u, low word: %u",
		(uint32_t)(sntp_time.seconds >> 32), (uint32_t)sntp_time.seconds);

	gmtime_r((time_t *)&sntp_time.seconds, (struct tm *)(&set_t));
	LOG_INF("Setting date/time is %d-%d-%d %d  %d:%d:%d\n", set_t.tm_year + 1900,
		set_t.tm_mon + 1, set_t.tm_mday, set_t.tm_wday, set_t.tm_hour, set_t.tm_min,
		set_t.tm_sec);

	ret = rtc_set_time(ds3231, &set_t);
	if (ret != 0) {
		LOG_INF("Error setting time!!");
		return false;
	}

	LOG_INF("Closing SNTP IPv4 ctx...");

	sntp_close(&ctx);

	return true;
}

int rtc_thread(void)
{

	ds3231 = get_ds3231_device();

	LOG_INF("Initializing rtc module");

	while (1) {

		bool updated = false;
		const struct zbus_channel *channel;

		readTimeAndPublish();

		updated = (zbus_sub_wait(&rtc_thread_subscriber, &channel, K_SECONDS(1)) == 0);
		if (!updated) {
			LOG_INF("Still waiting for Wifi status update");
			continue;
		}

		struct status_wifi_message wifi_status;
		int error = zbus_chan_read(channel, &wifi_status, K_FOREVER);
		if (error) {
			LOG_ERR("Error reading wifi status");
			continue;
		}

		if (!wifi_status.connected) {
			LOG_INF("Wifi not connected. Waiting for connection");
			continue;
		}

		LOG_INF("Wifi connected!");

		// mask the subscription to the wifi status channel as we are no more interested
		zbus_obs_set_chan_notification_mask(&rtc_thread_subscriber,
						    &status_wifi_channel, true);

		// Set the time from SNTP
		if (setRtcTimeFromSntp()) {
			LOG_INF("Time set from SNTP");
		} else {
			LOG_ERR("Error setting time from SNTP");
		}

		// define a variable to take the time, we need to refresh the time every 6 hours
		time_t last_time = 0;

		while (1) {

			readTimeAndPublish();

			// Check if we need to update the time
			time_t now = k_uptime_get() / 1000;
			if (now - last_time > 6 * 60 * 60) {
				if (setRtcTimeFromSntp()) {
					last_time = now;
					LOG_INF("Time set from SNTP");
				} else {
					LOG_ERR("Error setting time from SNTP");
				}
			}

			k_sleep(K_MSEC(CONFIG_RTC_FREQUENCY));
		}
	}

	return 0;
}

// Define the sensor threads
K_THREAD_DEFINE(rtc_thread_id, 2048, rtc_thread, NULL, NULL, NULL, 3, 0, 0);
