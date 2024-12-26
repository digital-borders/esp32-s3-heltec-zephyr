#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(USER_INTERFACE, LOG_LEVEL_ERR);

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>

#include "user_interface_zbus.h"

/**
 * The pwm pins information from the device tree
 */
static const struct pwm_dt_spec pwm = PWM_DT_SPEC_GET(DT_PATH(zephyr_user));

/**
 * Set the state of the buzzer
 */
static void set_buzzer(bool onOff)
{
	LOG_INF("Setting buzzer state to %d", onOff);

	int ret = 0;
	uint32_t buzzer_period = pwm.period;

	ret = onOff ? pwm_set_dt(&pwm, buzzer_period, buzzer_period / 2U)
		    : pwm_set_pulse_dt(&pwm, 0);

	if (ret < 0) {
		LOG_ERR("Failed to set buzzer state");
	}
}

/**
 * The user interface thread
 */
int user_interface_thread(void)
{
	LOG_INF("Initializing user interface module");

	// check if the pins are valid
	if (!pwm_is_ready_dt(&pwm)) {
		LOG_ERR("The pwm module %s is not ready", pwm.dev->name);
		return -EIO;
	}

	// start by turning off the buzzer
	set_buzzer(false);

	while (1) {
		// wait for a message on any actuator channel

		const struct zbus_channel *channel;

		zbus_sub_wait(&user_interface_thread_subscriber, &channel, K_FOREVER);

		LOG_INF("Received message on channel %s", zbus_chan_name(channel));

		// check the channel
		if (channel == &actuators_buzzer_channel) {
			struct actuators_buzzer_message message;
			zbus_chan_read(&actuators_buzzer_channel, &message, K_FOREVER);
			set_buzzer(message.onOff);
		}
	}

	return 0;
}

// Define the user interface threads
// K_THREAD_DEFINE(user_interface_thread_id, 2048, user_interface_thread, NULL, NULL, NULL, 3, 0,
// 0);
