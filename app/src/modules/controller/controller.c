
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(CNTRL, LOG_LEVEL_DBG);

#include <zephyr/data/json.h>
#include <time.h>

#include "controller_zbus.h"
#include "farming.h"

/* A structure holding sensors data shared between consuming and publishing threads*/
static struct {
	uint32_t time;
	float temperature;
	float humidity;
	float pressure;
	float co;
	int battery;
	int water_level;
} sensors_data;

/* Mutex for sharing data between data consumer thread and publishing thread */
K_MUTEX_DEFINE(controller_sensors_data_mutex);

/* A structure holding sensors data shared between consuming and publishing threads*/
static struct {
	bool water_pump_running;
	int water_pump_time_remaining;
	bool water_valve_open;
} actuators_data;

/* Mutex for sharing data between data consumer thread and publishing thread */
K_MUTEX_DEFINE(controller_actuators_data_mutex);

static struct farming farming_results;
K_MUTEX_DEFINE(controller_farming_mutex);

static struct config_farming_str_message farming_str_message;

int controller_management_thread(void)
{
	LOG_INF("Initializing controller management module");

	while (1) {
		// Controllo se ho actions da eseguire
		k_mutex_lock(&controller_sensors_data_mutex, K_FOREVER);
		uint32_t currentTime = sensors_data.time;
		k_mutex_unlock(&controller_sensors_data_mutex);

		k_mutex_lock(&controller_farming_mutex, K_FOREVER);
		struct farming current_farming_results;
		memcpy(&current_farming_results, &farming_results, sizeof(farming_results));
		k_mutex_unlock(&controller_farming_mutex);

		for (int i = 0; i < current_farming_results.num_elements; i++) {

			struct farming_action *action = &current_farming_results.actions[i];

			// Check if the action need to be evaluated (start and end)
			if (currentTime >= action->start && currentTime <= action->end) {

				// Check if the action is a time based action
				if (action->triggerType == 0) {

					// Check if we need to do something
					// in sensors_data.time we have time updated every 5 seconds
					// so we need to check if the current time is between
					// activationTime minus 5 seconds and activationTime plus 5
					// seconds

					// get hour of day from sensors_data.time
					struct tm *tm_info = localtime(&currentTime);
					int hour = tm_info->tm_hour;

					// LOG_INF("Current hour: %d", hour);

					if (hour == action->activationTime && !action->executed) {
						// execute the action

						LOG_INF("Execute action %d", i);

						if (action->actuatorId == 0) {
							// Water pump
							if (action->actionType == 0) {
								LOG_INF("Turn off water "
									"pump");
								// Off
								// compose message
								struct actuators_water_pump_message
									water_pump_running_message = {
										.run = false,
										.time_remaining = 0,
									};
								LOG_DBG("Publishing turn off water "
									"pump");
								zbus_chan_pub(
									&actuators_water_pump_channel,
									&water_pump_running_message,
									K_FOREVER);
							} else {
								// On
								LOG_INF("Turn on water "
									"pump");
								// compose message
								struct actuators_water_pump_message
									water_pump_running_message = {
										.run = true,
										.time_remaining =
											action->amount,
									};

								LOG_DBG("Publishing turn on water "
									"pump for %d seconds "
									"from controller",
									water_pump_running_message
										.time_remaining);
								zbus_chan_pub(
									&actuators_water_pump_channel,
									&water_pump_running_message,
									K_FOREVER);
							}
						} else {
							// Water valve
							if (action->actionType == 0) {
								// Off
								LOG_INF("Close water "
									"valve");
								struct actuators_water_valve_message
									water_valve_message = {
										.open = false,
									};

								LOG_DBG("Publishing close water "
									"valve from controller");
								zbus_chan_pub(
									&actuators_water_valve_channel,
									&water_valve_message,
									K_FOREVER);
							} else {
								// On
								LOG_INF("Open water valve");
								struct actuators_water_valve_message
									water_valve_message = {
										.open = true,
									};
								LOG_DBG("Publishing open water "
									"valve from controller");
								zbus_chan_pub(
									&actuators_water_valve_channel,
									&water_valve_message,
									K_FOREVER);
							}
						}
						// Set action as executed
						action->executed = true;
					}

				} else {
					// action is a sensor based action
				}
			}
		}

		k_sleep(K_MSEC(CONFIG_CONTROLLER_FREQUENCY));
	}

	return 0;
}

int controller_zbus_consumer_thread(void)
{
	LOG_INF("Initializing controller zbus consumer thread");

	while (1) {
		// wait for a message on read channel

		int error;

		const struct zbus_channel *channel;

		zbus_sub_wait(&controller_thread_subscriber, &channel, K_FOREVER);

		// LOG_DBG("Received message on channel %s", zbus_chan_name(channel));

		// check the channel
		if (channel == &config_farming_str_channel) {

			zbus_chan_read(&config_farming_str_channel, &farming_str_message,
				       K_FOREVER);

			LOG_DBG("Received farming config: %s",
				farming_str_message.farming_json_string);

			// PArse it

			int ret;
			k_mutex_lock(&controller_farming_mutex, K_FOREVER);

			/**
			 * Returns < 0 if error, bitmap of decoded fields on success (bit 0 is set
			 * if first field in the descriptor has been properly decoded, etc).
			 */
			ret = json_obj_parse(farming_str_message.farming_json_string,
					     sizeof(farming_str_message.farming_json_string),
					     farming_array_descr, ARRAY_SIZE(farming_array_descr),
					     &farming_results);

			k_mutex_unlock(&controller_farming_mutex);

			if (ret < 0) {
				LOG_ERR("JSON Parse Error on config_farming_str_message: %d", ret);
			} else {
				LOG_INF("json_obj_parse return code: %d", ret);

				// // log every element
				// for (int i = 0; i < farming_results.num_elements; i++) {
				// 	LOG_INF("start: %d", farming_results.actions[i].start);
				// 	LOG_INF("end: %d", farming_results.actions[i].end);
				// 	farming_results.actions[i].executed = false;
				// }
			}

		} else if (channel == &sensors_humidity_channel) {
			struct sensors_humidity_message humidity_message;
			error = zbus_chan_read(&sensors_humidity_channel, &humidity_message,
					       K_FOREVER);
			if (error < 0) {
				LOG_ERR("Error reading humidity sensor data");
				continue;
			}
			k_mutex_lock(&controller_sensors_data_mutex, K_FOREVER);
			sensors_data.humidity = humidity_message.humidity;
			k_mutex_unlock(&controller_sensors_data_mutex);

		} else if (channel == &sensors_pressure_channel) {
			static struct sensors_pressure_message pressure_message;
			error = zbus_chan_read(&sensors_pressure_channel, &pressure_message,
					       K_FOREVER);

			if (error < 0) {
				LOG_ERR("Error reading pressure sensor data");
				continue;
			}
			k_mutex_lock(&controller_sensors_data_mutex, K_FOREVER);
			sensors_data.pressure = pressure_message.pressure;
			k_mutex_unlock(&controller_sensors_data_mutex);

		} else if (channel == &sensors_co_channel) {
			struct sensors_co_message co_message;
			error = zbus_chan_read(&sensors_co_channel, &co_message, K_FOREVER);

			if (error < 0) {
				LOG_ERR("Error reading co sensor data");
				continue;
			}

			k_mutex_lock(&controller_sensors_data_mutex, K_FOREVER);
			sensors_data.co = co_message.co;
			k_mutex_unlock(&controller_sensors_data_mutex);

		} else if (channel == &sensors_temperature_channel) {
			struct sensors_temperature_message temperature_message;
			error = zbus_chan_read(&sensors_temperature_channel, &temperature_message,
					       K_FOREVER);

			if (error < 0) {
				LOG_ERR("Error reading temperature sensor data");
				continue;
			}
			k_mutex_lock(&controller_sensors_data_mutex, K_FOREVER);
			sensors_data.temperature = temperature_message.temperature;
			k_mutex_unlock(&controller_sensors_data_mutex);

		} else if (channel == &sensors_water_level_channel) {
			struct sensors_water_level_message water_level_message;
			error = zbus_chan_read(&sensors_water_level_channel, &water_level_message,
					       K_FOREVER);
			if (error < 0) {
				LOG_ERR("Error reading water level sensor data");
				continue;
			}
			LOG_DBG("water_level_message: %d", water_level_message.percentage);
			k_mutex_lock(&controller_sensors_data_mutex, K_FOREVER);
			sensors_data.water_level = water_level_message.percentage;
			k_mutex_unlock(&controller_sensors_data_mutex);
		} else if (channel == &status_water_pump_channel) {
			struct actuators_water_pump_message water_pump_running_message;
			error = zbus_chan_read(&status_water_pump_channel,
					       &water_pump_running_message, K_FOREVER);

			if (error < 0) {
				LOG_ERR("Error reading water pump actuator data");
				continue;
			}

			LOG_DBG("water_pump_running_message: %d",
				water_pump_running_message.time_remaining);

			k_mutex_lock(&controller_actuators_data_mutex, K_FOREVER);
			actuators_data.water_pump_running = water_pump_running_message.run;
			actuators_data.water_pump_time_remaining =
				water_pump_running_message.time_remaining;
			k_mutex_unlock(&controller_actuators_data_mutex);

		} else if (channel == &status_water_valve_channel) {
			struct actuators_water_valve_message water_valve_message = {.open = false};
			error = zbus_chan_read(&status_water_valve_channel, &water_valve_message,
					       K_FOREVER);
			if (error < 0) {
				LOG_ERR("Error reading water valve actuator data");
				continue;
			}

			LOG_DBG("water_valve_message: %d", (int)water_valve_message.open);
			k_mutex_lock(&controller_actuators_data_mutex, K_FOREVER);
			actuators_data.water_valve_open = water_valve_message.open;
			k_mutex_unlock(&controller_actuators_data_mutex);

		} else if (channel == &sensors_rtc_channel) {
			struct sensors_rtc_message rtc_message;
			error = zbus_chan_read(&sensors_rtc_channel, &rtc_message, K_FOREVER);

			if (error < 0) {
				LOG_ERR("Error reading rtc sensor data");
				continue;
			}
			k_mutex_lock(&controller_sensors_data_mutex, K_FOREVER);
			sensors_data.time = rtc_message.time;
			k_mutex_unlock(&controller_sensors_data_mutex);
		}
	}

	return 0;
}

// Define the sensor threads
K_THREAD_DEFINE(controller_management_thread_id, 2048, controller_management_thread, NULL, NULL,
		NULL, 3, 0, 0);
K_THREAD_DEFINE(controller_zbus_consumer_thread_id, 4096, controller_zbus_consumer_thread, NULL,
		NULL, NULL, 3, 0, 0);
