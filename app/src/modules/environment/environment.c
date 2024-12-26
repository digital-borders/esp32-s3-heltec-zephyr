#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ENVIRONMENT, LOG_LEVEL_ERR);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

#include "./devices/bme280.h"

#include "environment_zbus.h"

// Define a semaphorre for BME280 sharing. This will be set available after inizialization
static K_SEM_DEFINE(bme280_mutex, 0, 1);

// The BME280 device
static const struct device *bme280;

int environment_inizialization_thread(void)
{
	LOG_INF("Initializing environment module");

	bme280 = get_bme280_device();

	// check if the device is valid
	if (bme280 == NULL) {
		LOG_ERR("Could not initialize BME280 sensor");
		return -1;
	}

	// give the semaphore to allow the other threads to access the BME280 sensor
	k_sem_give(&bme280_mutex);

	return 0;
}

void environment_temperature_thread(void)
{
	char tag[] = "[TemperatureThread]:";
	while (1) {
		LOG_INF("%s Trying to acquire semaphore", tag);
		// try to gain the access to the BME280 sensor
		k_sem_take(&bme280_mutex, K_FOREVER);

		LOG_INF("%s Got semaphore", tag);

		// read the temperature
		struct sensor_value temperature;
		int ret;
		ret = sensor_sample_fetch(bme280);
		ret = sensor_channel_get(bme280, SENSOR_CHAN_AMBIENT_TEMP, &temperature);

		LOG_INF("%s Read sensor complete", tag);

		// release the access to the BME280 sensor
		k_sem_give(&bme280_mutex);

		LOG_INF("%s Released semaphore", tag);
		float temp = sensor_value_to_float(&temperature);

		// create the data message
		struct sensors_temperature_message message = {.temperature = temp};

		// publish the data
		LOG_DBG("Publishing temperature = %.1f.", (double)message.temperature);

		zbus_chan_pub(&sensors_temperature_channel, &message, K_FOREVER);

		LOG_DBG("Published temperature");

		// wait for the next iteration
		k_sleep(K_MSEC(CONFIG_SENSORS_TEMPERATURE_FREQUENCY));
	}
}

void environment_humidity_thread(void)
{
	char tag[] = "[HumidityThread]:";

	while (1) {

		LOG_INF("%s Trying to acquire semaphore", tag);

		// try to gain the access to the BME280 sensor
		k_sem_take(&bme280_mutex, K_FOREVER);

		LOG_INF("%s Got semaphore", tag);

		// read the humidity
		struct sensor_value humidity;
		int ret;
		ret = sensor_sample_fetch(bme280);
		ret = sensor_channel_get(bme280, SENSOR_CHAN_HUMIDITY, &humidity);

		LOG_INF("%s Read sensor complete", tag);

		// release the access to the BME280 sensor
		k_sem_give(&bme280_mutex);

		LOG_INF("%s Released semaphore", tag);

		float hum = sensor_value_to_float(&humidity);

		// create the data message
		struct sensors_humidity_message message = {.humidity = hum};

		// publish the data
		LOG_DBG("Publishing humidity = %.1f.", (double)message.humidity);

		zbus_chan_pub(&sensors_humidity_channel, &message, K_FOREVER);

		LOG_DBG("Published humidity");

		// wait for the next iteration
		k_sleep(K_MSEC(CONFIG_SENSORS_HUMIDITY_FREQUENCY));
	}
}

void environment_pressure_thread(void)
{
	char tag[] = "[PressureThread]:";

	while (1) {
		LOG_INF("%s Trying to acquire semaphore", tag);

		// try to gain the access to the BME280 sensor
		k_sem_take(&bme280_mutex, K_FOREVER);

		LOG_INF("%s Got semaphore", tag);

		// read the pressure in kilopascal
		struct sensor_value pressure;
		int ret;
		ret = sensor_sample_fetch(bme280);
		ret = sensor_channel_get(bme280, SENSOR_CHAN_PRESS, &pressure);
		LOG_INF("%s Read sensor complete", tag);
		// release the access to the BME280 sensor
		k_sem_give(&bme280_mutex);

		LOG_INF("%s Released semaphore", tag);
		float press = sensor_value_to_float(&pressure);
		// create the data message
		struct sensors_pressure_message message = {.pressure = press};

		// publish the data
		LOG_DBG("Publishing pressure = %.1f.", (double)message.pressure);

		zbus_chan_pub(&sensors_pressure_channel, &message, K_FOREVER);

		LOG_DBG("Published pressure");

		// wait for the next iteration
		k_sleep(K_MSEC(CONFIG_SENSORS_PRESSURE_FREQUENCY));
	}
}

// #define DT_SPEC_AND_COMMA(node_id, prop, idx) ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

// /* Data of ADC io-channels specified in devicetree. */
// // static const struct adc_dt_spec adc_channels[] = {
// // 	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA)};

// static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

// static struct sensors_co_message co_message = {.co = 0};

// int environment_co_thread(void)
// {
// 	int err;
// 	uint32_t count = 0;
// 	uint16_t buf;
// 	struct adc_sequence sequence = {
// 		.buffer = &buf,
// 		/* buffer size in bytes, not number of samples */
// 		.buffer_size = sizeof(buf),
// 	};

// 	/* Configure channels individually prior to sampling. */

// 	if (!adc_is_ready_dt(&adc_channel)) {
// 		LOG_ERR("ADC controller device %s not ready", adc_channel.dev->name);
// 		return 0;
// 	}

// 	err = adc_channel_setup_dt(&adc_channel);
// 	if (err < 0) {
// 		LOG_ERR("Could not setup channel #%d (%d)", 5, err);
// 		return 0;
// 	}

// 	int32_t median_val_mv = 0;

// 	while (1) {

// 		for (int k = 0; k < 10; k++) {

// 			// LOG_DBG("ADC reading[%u]:", count++);

// 			int32_t val_mv;

// 			// LOG_DBG("- %s, channel %d: ", adc_channel.dev->name,
// 			// 	adc_channel.channel_id);

// 			(void)adc_sequence_init_dt(&adc_channel, &sequence);

// 			err = adc_read_dt(&adc_channel, &sequence);
// 			if (err < 0) {
// 				LOG_ERR("Could not read (%d)", err);
// 				continue;
// 			}

// 			/*
// 			 * If using differential mode, the 16 bit value
// 			 * in the ADC sample buffer should be a signed 2's
// 			 * complement value.
// 			 */
// 			if (adc_channel.channel_cfg.differential) {
// 				val_mv = (int32_t)((int16_t)buf);
// 			} else {
// 				val_mv = (int32_t)buf;
// 			}
// 			// LOG_DBG("%" PRId32, val_mv);

// 			median_val_mv += val_mv;

// 			err = adc_raw_to_millivolts_dt(&adc_channel, &val_mv);
// 			/* conversion to mV may not be supported, skip if not */
// 			if (err < 0) {
// 				LOG_ERR(" (value in mV not available) ");
// 			} else {
// 				// LOG_DBG(" = %" PRId32 " mV", val_mv);
// 			}

// 			k_sleep(K_MSEC(1000));
// 		}

// 		median_val_mv /= 10;

// 		// create the data message
// 		co_message.co = median_val_mv;

// 		LOG_DBG("Publishing co = %d.", (int)co_message.co);

// 		zbus_chan_pub(&sensors_co_channel, &co_message, K_FOREVER);

// 		LOG_DBG("Published co");
// 	}
// 	return 0;
// }

// Define the sensor threads
K_THREAD_DEFINE(environment_inizialization_thread_id, 1024, environment_inizialization_thread, NULL,
		NULL, NULL, 3, 0, 0);

K_THREAD_DEFINE(environment_temperature_thread_id, 1024, environment_temperature_thread, NULL, NULL,
		NULL, 3, 0, 0);
K_THREAD_DEFINE(environment_humidity_thread_id, 1024, environment_humidity_thread, NULL, NULL, NULL,
		3, 0, 0);
K_THREAD_DEFINE(environment_pressure_thread_id, 1024, environment_pressure_thread, NULL, NULL, NULL,
		3, 0, 0);
// K_THREAD_DEFINE(environment_co_thread_id, 2048, environment_co_thread, NULL, NULL, NULL, 3, 0,
// 0);