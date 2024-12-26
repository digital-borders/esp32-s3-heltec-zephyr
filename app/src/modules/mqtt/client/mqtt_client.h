#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>

/// @brief MQTT subscription callback function
/// @param topic_name the topic name (null terminated)
/// @param payload the received data
/// @param payload_length the length of the received data
typedef void (*callback_t)(uint8_t *topic_name, uint8_t *payload, uint16_t payload_length);

/// @brief Initialise the MQTT client & broker configuration
/// @param client the MQTT client instance
/// @param client_id the MQTT client ID
/// @return 0 on success, negative errno on error
int app_mqtt_init(struct mqtt_client *client, const uint8_t *client_id);

/// @brief Blocking function that establishes connectivity to the MQTT broker
/// @param client the MQTT client instance
void app_mqtt_connect(struct mqtt_client *client);

/// @brief Blocking function for incoming data processing. Returns if connection fails.
/// @param client the MQTT client instance
void app_mqtt_run(struct mqtt_client *client);

/// @brief Publish a message to the MQTT broker
/// @param client the MQTT client instance
/// @param topic the MQTT topic
/// @param payload the payload
void app_mqtt_publish(struct mqtt_client *client, struct mqtt_topic *topic,
		      struct mqtt_binstr *payload);

/// @brief Subscribe to a topic
/// @param client the MQTT client instance
/// @param topic the MQTT topic to subscribe to
/// @param callback the callback function to be called when a message is received
void app_mqtt_subscribe(struct mqtt_client *client, struct mqtt_topic *topic, callback_t callback);

/// @brief Check if the MQTT client is connected
/// @param None
/// @return true if connected, false otherwise
bool app_mqtt_connected(void);

#endif /* MQTT_CLIENT_H */