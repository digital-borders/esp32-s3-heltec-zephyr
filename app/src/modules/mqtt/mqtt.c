#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MQTT, LOG_LEVEL_ERR);

#include <zephyr/kernel.h>
#include <string.h>
#include "client/mqtt_client.h"
#include "mqtt_zbus.h"

/* MQTT client struct */
static struct mqtt_client client_ctx;

/* MQTT payload buffer used to compose the message to be published */
static uint8_t payload_buf[CONFIG_MQTT_PAYLOAD_BUFFER_SIZE];

/* MQTT publish work item */
struct k_work_delayable mqtt_publish_work;

/* A structure holding sensors data shared between consuming and publishing threads*/
static struct {
	float temperature;
	float humidity;
	float pressure;
	float co;
	int battery;
	int water_level;
} sensors_data;

/* Mutex for sharing data between data consumer thread and publishing thread */
K_MUTEX_DEFINE(sensors_data_mutex);

/* A structure holding sensors data shared between consuming and publishing threads*/
static struct {
	bool water_pump_running;
	int water_pump_time_remaining;
	bool water_valve_open;
} actuators_data;

/* Mutex for sharing data between data consumer thread and publishing thread */
K_MUTEX_DEFINE(actuators_data_mutex);

static struct status_wifi_message wifi_status = {
	.connected = false,
};

static struct info_device_message device_info_message = {.device_uuid = {0}};

/* Mutex for sharing data between data consumer thread and publishing thread */
K_MUTEX_DEFINE(status_data_mutex);

// DEVICE NAME
static char dev_name[32];

// message to sent
static const struct config_farming_str_message farming_str_message_to_sent;

/// @brief A TEST CALLBACK TO BE ADJUSTED!
void test_callback(uint8_t *topic_name, uint8_t *payload, uint16_t payload_length)
{
	LOG_INF("Received payload: %s with length %d", payload, payload_length);
	LOG_INF("from topic: %s", topic_name);

	// TODO just for test!!!!!!!
	// if the topic include the device id f412fa689b4c then do something
	if (strstr(topic_name, "f412fa689b4c") != NULL) {
		LOG_INF("Received data for device f412fa689b4c");

		// Check if topic contains set-farming
		if (strstr(topic_name, "set-farming") != NULL) {
			LOG_INF("Received data for set farming");

			// TODO: Just send a config_farming_str_message to the controller
			sprintf((char *)farming_str_message_to_sent.farming_json_string, "%.*s",
				payload_length, payload);
			LOG_DBG("Publishing farming: %s from mqtt",
				farming_str_message_to_sent.farming_json_string);
			zbus_chan_pub(&config_farming_str_channel, &farming_str_message_to_sent, K_FOREVER);
			LOG_DBG("Published farming config");
		}

	} else {
		LOG_INF("Received data for another device");
		// TODO: Need to send this to the correct lora device
	}
}

/// @brief  Wait for wifi connection to be established.
/// @param None
static void wait_for_wifi(void)
{
	LOG_INF("Waiting for wifi connection");
	bool wifiConnected = false;
	while (1) {
		// wait for the wifi status to change for a maximum of 10 seconds
		LOG_INF("Trying to lock mutex");
		k_mutex_lock(&status_data_mutex, K_FOREVER);
		LOG_INF("Mutex locked");
		wifiConnected = wifi_status.connected;
		k_mutex_unlock(&status_data_mutex);
		LOG_INF("Mutex unlocked - wifi status: %d", wifiConnected);

		// check if wifi is connected
		if (wifiConnected) {
			LOG_INF("Wifi connected!");
			break;
		}
		k_sleep(K_SECONDS(1));
	}
}

/// @brief  Wait for wifi connection to be established.
/// @param None
static void wait_for_device_id(void)
{
	LOG_INF("Waiting for device id");

	int length = 0;
	while (1) {
		// wait for the device id to be received
		k_mutex_lock(&status_data_mutex, K_FOREVER);
		length = strlen(device_info_message.device_uuid);
		k_mutex_unlock(&status_data_mutex);

		if (length > 0) {
			LOG_INF("Device id received!");
			break;
		}
		k_sleep(K_SECONDS(1));
	}
}

/// @brief Compose the sensor payload in JSON format
/// @param payload The buffer to store the payload
static void compose_sensor_payload(uint8_t *payload)
{
	// todo compose the payload
	k_mutex_lock(&sensors_data_mutex, K_FOREVER);
	sprintf(payload,
		"{\"temperature\":%.2f,\"humidity\":%d,\"pressure\":%.2f, \"co\":%.2f, "
		"\"battery\":%d, \"waterLevel\":%d}",
		sensors_data.temperature, (int)(sensors_data.humidity), sensors_data.pressure,
		sensors_data.co, sensors_data.battery, sensors_data.water_level);
	k_mutex_unlock(&sensors_data_mutex);
}

/// @brief Publish the sensors data to the MQTT broker
/// @param  None
static void publish_sensors_data(void)
{

	// construct pub topic like this thf/<dev_name>/sensors
	char pub_topic[64];
	sprintf(pub_topic, "thf/%s/sensors", dev_name);

	// configure the topic
	struct mqtt_topic topic = {.topic = {.utf8 = pub_topic, .size = strlen(topic.topic.utf8)},
				   .qos = MQTT_QOS_1_AT_LEAST_ONCE};

	// compose the payload
	struct mqtt_binstr payload;
	compose_sensor_payload(payload_buf);
	payload.data = payload_buf;
	payload.len = strlen(payload.data);

	app_mqtt_publish(&client_ctx, &topic, &payload);
}

/// @brief The handler for MQTT publish work
/// @param work The work item
static void publish_work_handler(struct k_work *work)
{
	if (app_mqtt_connected()) {
		publish_sensors_data();
		k_work_reschedule(&mqtt_publish_work, K_SECONDS(CONFIG_MQTT_PUBLISH_INTERVAL_S));
	} else {
		k_work_cancel_delayable(&mqtt_publish_work);
	}
}

/// @brief Subscribe to the MQTT topics
/// @param None
static void subscribe_to_mqtt_topics(void)
{
	// subscribe to the test topic (TODO)
	struct mqtt_topic sub_topic = {
		.topic = {.utf8 = CONFIG_MQTT_SUB_TOPIC, .size = strlen(CONFIG_MQTT_SUB_TOPIC)},
		.qos = MQTT_QOS_1_AT_LEAST_ONCE};

	app_mqtt_subscribe(&client_ctx, &sub_topic, test_callback);
}

/// @brief The mqtt data consumer thread for consuming sensor data
/// @param  None
/// @return The thread return code
static int mqtt_zbus_consumer_thread(void)
{
	while (1) {
		// wait for the sensor data
		const struct zbus_channel *channel;
		int error =
			zbus_sub_wait(&mqtt_zbus_consumer_thread_subscriber, &channel, K_FOREVER);
		if (error) {
			LOG_ERR("Error waiting for sensors data");
			continue;
		}

		// check the channel
		if (channel == &sensors_temperature_channel) {
			// read the temperature sensor data
			struct sensors_temperature_message temperature_message;
			error = zbus_chan_read(&sensors_temperature_channel, &temperature_message,
					       K_FOREVER);
			if (error) {
				LOG_ERR("Error reading temperature sensor data");
				continue;
			}

			// store the temperature sensor data
			k_mutex_lock(&sensors_data_mutex, K_FOREVER);
			sensors_data.temperature = temperature_message.temperature;
			k_mutex_unlock(&sensors_data_mutex);
		} else if (channel == &sensors_humidity_channel) {
			// read the temperature sensor data
			struct sensors_humidity_message humidity_message;
			error = zbus_chan_read(&sensors_humidity_channel, &humidity_message,
					       K_FOREVER);
			if (error) {
				LOG_ERR("Error reading humidity sensor data");
				continue;
			}

			// store the temperature sensor data
			k_mutex_lock(&sensors_data_mutex, K_FOREVER);
			sensors_data.humidity = humidity_message.humidity;
			k_mutex_unlock(&sensors_data_mutex);
		} else if (channel == &sensors_pressure_channel) {
			// read the temperature sensor data
			struct sensors_pressure_message pressure_message;
			error = zbus_chan_read(&sensors_pressure_channel, &pressure_message,
					       K_FOREVER);
			if (error) {
				LOG_ERR("Error reading pressure sensor data");
				continue;
			}

			// store the temperature sensor data
			k_mutex_lock(&sensors_data_mutex, K_FOREVER);
			sensors_data.pressure = pressure_message.pressure;
			k_mutex_unlock(&sensors_data_mutex);
		} else if (channel == &sensors_co_channel) {
			// read the temperature sensor data
			struct sensors_co_message co_message;
			error = zbus_chan_read(&sensors_co_channel, &co_message, K_FOREVER);
			if (error) {
				LOG_ERR("Error reading co sensor data");
				continue;
			}

			// store the temperature sensor data
			k_mutex_lock(&sensors_data_mutex, K_FOREVER);
			sensors_data.co = co_message.co;
			k_mutex_unlock(&sensors_data_mutex);
		} else if (channel == &sensors_battery_channel) {
			// read the battery sensor data
			struct sensors_battery_message battery_message;
			error = zbus_chan_read(&sensors_battery_channel, &battery_message,
					       K_FOREVER);
			if (error) {
				LOG_ERR("Error reading battery sensor data");
				continue;
			}

			// store the temperature sensor data
			k_mutex_lock(&sensors_data_mutex, K_FOREVER);
			sensors_data.battery = battery_message.percentage;
			k_mutex_unlock(&sensors_data_mutex);
		} else if (channel == &sensors_water_level_channel) {
			// read the water level sensor data
			struct sensors_water_level_message water_level_message;
			error = zbus_chan_read(&sensors_water_level_channel, &water_level_message,
					       K_FOREVER);
			if (error) {
				LOG_ERR("Error reading water level sensor data");
				continue;
			}

			// store the temperature sensor data
			k_mutex_lock(&sensors_data_mutex, K_FOREVER);
			sensors_data.water_level = water_level_message.percentage;
			k_mutex_unlock(&sensors_data_mutex);
		} else if (channel == &actuators_water_pump_channel) {
			// read the water pump actuator data
			struct actuators_water_pump_message water_pump_message;
			error = zbus_chan_read(&actuators_water_pump_channel, &water_pump_message,
					       K_FOREVER);
			if (error) {
				LOG_ERR("Error reading water pump actuator data");
				continue;
			}

			// store the water pump actuator data
			k_mutex_lock(&actuators_data_mutex, K_FOREVER);
			actuators_data.water_pump_running = water_pump_message.run;
			actuators_data.water_pump_time_remaining =
				water_pump_message.time_remaining;
			k_mutex_unlock(&actuators_data_mutex);
		} else if (channel == &actuators_water_valve_channel) {
			// read the water valve actuator data
			struct actuators_water_valve_message water_valve_message;
			error = zbus_chan_read(&actuators_water_valve_channel, &water_valve_message,
					       K_FOREVER);
			if (error) {
				LOG_ERR("Error reading water valve actuator data");
				continue;
			}

			// store the water valve actuator data
			k_mutex_lock(&actuators_data_mutex, K_FOREVER);
			actuators_data.water_valve_open = water_valve_message.open;
			k_mutex_unlock(&actuators_data_mutex);
		} else if (channel == &status_wifi_channel) {
			// read the wifi status data
			LOG_INF("[consumer]: Reading wifi status data");
			k_mutex_lock(&status_data_mutex, K_FOREVER);
			LOG_INF("[consumer]: Locked mutex");
			error = zbus_chan_read(&status_wifi_channel, &wifi_status, K_FOREVER);
			LOG_INF("[consumer]: Read wifi status data: %d", wifi_status.connected);
			k_mutex_unlock(&status_data_mutex);
			LOG_INF("[consumer]: Unlocked mutex");
			if (error) {
				LOG_ERR("Error reading wifi status data");
				continue;
			}

		} else if (channel == &info_device_channel) {
			// read the device info data

			k_mutex_lock(&status_data_mutex, K_FOREVER);
			error = zbus_chan_read(&info_device_channel, &device_info_message,
					       K_FOREVER);
			k_mutex_unlock(&status_data_mutex);
			if (error) {
				LOG_ERR("Error reading device info data");
				continue;
			}

		} else {
			LOG_ERR("Unknown sensor channel");
		}
	}

	return 0;
}

/// @brief The mqtt management thread for connection management
/// @param  None
/// @return The thread return code
static int mqtt_management_thread(void)
{
	LOG_INF("Initializing mqtt module");

	bool subscribed = false;

	wait_for_device_id();

	k_mutex_lock(&status_data_mutex, K_FOREVER);
	sprintf(dev_name, "jocondo-%02x%02x%02x%02x%02x%02x", device_info_message.device_uuid[0],
		device_info_message.device_uuid[1], device_info_message.device_uuid[2],
		device_info_message.device_uuid[3], device_info_message.device_uuid[4],
		device_info_message.device_uuid[5]);
	k_mutex_unlock(&status_data_mutex);

	while (1) {
		wait_for_wifi();

		LOG_INF("Wifi connection detected");

		int error = app_mqtt_init(&client_ctx, dev_name);
		if (error) {
			LOG_ERR("MQTT Init failed with error %d", error);
			continue;
		}

		/* Initialise MQTT publish work item */
		k_work_init_delayable(&mqtt_publish_work, publish_work_handler);

		/* Thread main loop */
		while (1) {
			/* Block until MQTT connection is up */
			LOG_INF("Connecting to MQTT broker");
			app_mqtt_connect(&client_ctx);

			/* Subscribe to the test topic */
			if (!subscribed) {
				subscribe_to_mqtt_topics();
				subscribed = true;
				LOG_INF("Subscribed to MQTT topics");
			}

			/* We are now connected, begin queueing periodic MQTT publishes */
			k_work_reschedule(&mqtt_publish_work,
					  K_SECONDS(CONFIG_MQTT_PUBLISH_INTERVAL_S));

			/* Handle MQTT inputs and connection */
			app_mqtt_run(&client_ctx);
			LOG_INF("Disconnected from MQTT broker");
			wait_for_wifi();
		}
	}

	return 0;
}

// Define the mqtt threads
K_THREAD_DEFINE(mqtt_management_thread_id, 4096, mqtt_management_thread, NULL, NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(mqtt_zbus_consumer_thread_id, 1024, mqtt_zbus_consumer_thread, NULL, NULL, NULL, 3,
		0, 0);