#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(WATER_MANAGEMENT, LOG_LEVEL_ERR);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#include "water_management_zbus.h"

static struct actuators_water_pump_message water_pump_message = {.run = false, .time_remaining = 0};

/**
 * The water pump gpio pins information from the device tree
 */
static const struct gpio_dt_spec water_pump = GPIO_DT_SPEC_GET(DT_NODELABEL(pumprelay), gpios);

/**
 * The water valve gpio pins information from the device tree
 */
static const struct gpio_dt_spec water_valve = GPIO_DT_SPEC_GET(DT_NODELABEL(valverelay), gpios);

/**
 * Set the state of the water pump
 */
static void set_pump(bool run, uint32_t time_remaining)
{
	LOG_INF("Setting pump state to %d for %d seconds", run, time_remaining);
	int ret = gpio_pin_set_dt(&water_pump, run);
	if (ret < 0) {
		LOG_ERR("Failed to set pump state");
	}
	struct status_water_pump_message message = {.running = run};
	LOG_DBG("Publishing pump state");
	zbus_chan_pub(&status_water_pump_channel, &message, K_FOREVER);
}

/**
 * Set the state of the water valve
 */
static void set_valve(bool open)
{
	int ret = gpio_pin_set_dt(&water_valve, open);
	if (ret < 0) {
		LOG_ERR("Failed to set valve state");
	}
	struct status_water_valve_message message = {.opened = open};
	LOG_INF("Setting valve state to %d", open);
	LOG_DBG("Publishing valve state");
	zbus_chan_pub(&status_water_valve_channel, &message, K_FOREVER);
}

/**
 * The water management thread
 */
int water_management_thread(void)
{
	// disable observer until initialization is complete
	zbus_obs_set_enable(&water_management_thread_subscriber, false);

	LOG_INF("Initializing water management module");

	// check if the pins are valid
	if (!device_is_ready(water_pump.port) || !device_is_ready(water_valve.port)) {
		LOG_ERR("Invalid water pump or valve pins");
		return -EIO;
	}

	// configure the pins
	int ret = 0;
	ret |= gpio_pin_configure_dt(&water_pump, GPIO_OUTPUT_INACTIVE);
	ret |= gpio_pin_configure_dt(&water_valve, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		LOG_ERR("Failed to configure water pump or valve pins");
		return -EIO;
	}

	// start by turning off the pump and closing the valve. This will notifiy the status
	// channels
	set_pump(false, 0);
	set_valve(false);

	zbus_obs_set_enable(&water_management_thread_subscriber, true);

	while (1) {
		// wait for a message on any actuator channel
		int updated = 0;
		const struct zbus_channel *channel;

		updated = zbus_sub_wait(&water_management_thread_subscriber, &channel, K_NO_WAIT);

		if (updated == 0) {

			LOG_INF("Received message on channel %s", zbus_chan_name(channel));

			// check the channel
			if (channel == &actuators_water_pump_channel) {
				zbus_chan_read(&actuators_water_pump_channel, &water_pump_message,
					       K_FOREVER);
				LOG_INF("Received water pump message with run: %d and time "
					"remaining: %d",
					water_pump_message.run, water_pump_message.time_remaining);
				set_pump(water_pump_message.run, water_pump_message.time_remaining);

			} else if (channel == &actuators_water_valve_channel) {
				struct actuators_water_valve_message message;
				zbus_chan_read(&actuators_water_valve_channel, &message, K_FOREVER);
				set_valve(message.open);
			}
		}

		// if there is some time remaining, substract one second
		if (water_pump_message.run) {
			water_pump_message.time_remaining--;
			if (water_pump_message.time_remaining == 0) {
				set_pump(false, 0);
				water_pump_message.run = false;
			}
		}

		// wait for the next iteration
		k_sleep(K_MSEC(1000));
	}

	return 0;
}

// Define the water management threads
// K_THREAD_DEFINE(water_management_thread_id, 2048, water_management_thread, NULL, NULL, NULL, 3,
// 0, 0);
