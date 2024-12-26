#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MQTT_CLIENT, LOG_LEVEL_ERR);

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/data/json.h>
#include <zephyr/random/random.h>

#include "mqtt_client.h"

/* Buffers for MQTT client */
static uint8_t rx_buffer[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
static uint8_t tx_buffer[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];

/* MQTT broker details */
static struct sockaddr_storage broker;

/* Socket descriptor */
static struct zsock_pollfd fds[1];

/* MQTT connectivity status flag */
static bool mqtt_connected;

/* MQTT authentication details */
static struct mqtt_utf8 mqtt_user_name = MQTT_UTF8_LITERAL(CONFIG_MQTT_USERNAME);
static struct mqtt_utf8 mqtt_password = MQTT_UTF8_LITERAL(CONFIG_MQTT_PASSWORD);

/* The list of subscriptions */
static struct {
	struct mqtt_topic topic;
	callback_t callback;
} subscriptions[CONFIG_MQTT_MAXIMUM_SUBSCRIPTIONS];

/* Used to split the topic name in slices */
typedef struct {
	uint8_t *slice;
	size_t size;
} slice_t;

/// @brief Handler for the MQTT connect event
/// @param None
static inline void on_mqtt_connect(void)
{
	mqtt_connected = true;
	LOG_INF("Connected to MQTT broker!");
}

/// @brief Handler for the MQTT disconnect event
/// @param None
static inline void on_mqtt_disconnect(void)
{
	mqtt_connected = false;
	LOG_INF("Disconnected from MQTT broker");
}

/// @brief Split the topic name into slices
/// @param topic_name The topic name to split
/// @param topic_name_length The length of the topic name
/// @param slices The slices to store the split topic name
/// @param slice_count The number of slices
void split_topic_name(const char *topic_name, size_t topic_name_length, slice_t *slices,
		      size_t *slice_count)
{
	const char *start = topic_name;
	const char *end = topic_name;
	size_t count = 0;

	while (*end != '\0') {
		if (*end == '/') {
			slices[count].slice = (uint8_t *)start;
			slices[count].size = end - start;
			count++;
			start = end;
			start++;
		}
		end++;
	}

	slices[count].slice = (uint8_t *)start;
	slices[count].size = end - start;
	count++;

	*slice_count = count;
}

/// @brief Compare the topic slices taking in account the wildcards
/// @param subscription_topic_slices The subscription topic slices
/// @param subscription_topic_slice_count The number of subscription topic slices
/// @param event_topic_slices The event topic slices
/// @param event_topic_slice_count The number of event topic slices
bool compare_topics_slices(slice_t *subscription_topic_slices,
			   size_t subscription_topic_slice_count, slice_t *event_topic_slices,
			   size_t event_topic_slice_count)
{
	// compare the event topic slices with the subscription topic slices
	// takeing in account the wildcards
	bool match = true;
	for (int j = 0; j < event_topic_slice_count; j++) {
		// event is a subtopic of the subscription
		if (j >= subscription_topic_slice_count) {
			match = false;
			break;
		}

		if (subscription_topic_slices[j].size == 1 &&
		    subscription_topic_slices[j].slice[0] == '+') {
			// ignore the subtopic
			continue;
		}

		if (subscription_topic_slices[j].size == 1 &&
		    subscription_topic_slices[j].slice[0] == '#') {
			// ignore the rest of the slices
			break;
		}

		if (event_topic_slices[j].size != subscription_topic_slices[j].size) {
			match = false;
			break;
		}

		if (memcmp(event_topic_slices[j].slice, subscription_topic_slices[j].slice,
			   event_topic_slices[j].size) != 0) {
			match = false;
			break;
		}
	}

	return match;
}

/// @brief Handler for the MQTT publish event. Called when a payload is received
/// @param client the MQTT client instance
/// @param evt the MQTT event
static void on_mqtt_publish(struct mqtt_client *const client, const struct mqtt_evt *evt)
{
	int payload_length;
	uint8_t payload[CONFIG_MQTT_PAYLOAD_BUFFER_SIZE];

	payload_length =
		mqtt_read_publish_payload(client, payload, CONFIG_MQTT_PAYLOAD_BUFFER_SIZE);
	if (payload_length < 0) {
		LOG_ERR("Read publish event payload failed with error: %d", payload_length);
		return;
	}

	// Place null terminator at end of payload buffer
	payload[payload_length] = '\0';

	LOG_INF("MQTT payload received!");
	LOG_INF("topic: '%.*s', payload: %s", evt->param.publish.message.topic.topic.size,
		evt->param.publish.message.topic.topic.utf8, payload);

	size_t event_topic_slice_count;
	slice_t event_topic_slices[CONFIG_MQTT_MAXIMUM_TOPIC_SLICES];

	size_t subscription_topic_slice_count;
	slice_t subscription_topic_slices[CONFIG_MQTT_MAXIMUM_TOPIC_SLICES];

	// split the event topic name into slices
	split_topic_name(evt->param.publish.message.topic.topic.utf8,
			 evt->param.publish.message.topic.topic.size, event_topic_slices,
			 &event_topic_slice_count);

	// call the appropriate subscription callback
	for (int i = 0; i < CONFIG_MQTT_MAXIMUM_SUBSCRIPTIONS; i++) {
		if (subscriptions[i].callback == NULL) {
			continue;
		}

		// split the subscription topic name into slices
		split_topic_name(subscriptions[i].topic.topic.utf8,
				 subscriptions[i].topic.topic.size, subscription_topic_slices,
				 &subscription_topic_slice_count);

		// compare the topic slices and call the callback if they match
		if (compare_topics_slices(subscription_topic_slices, subscription_topic_slice_count,
					  event_topic_slices, event_topic_slice_count)) {

			// compute the topic name
			if (evt->param.publish.message.topic.topic.size >
			    CONFIG_MQTT_MAXIMUM_TOPIC_NAME_LENGTH) {
				LOG_ERR("Topic name too long");
				return;
			}

			uint8_t topic_name[CONFIG_MQTT_MAXIMUM_TOPIC_NAME_LENGTH];

			memcpy(topic_name, evt->param.publish.message.topic.topic.utf8,
			       evt->param.publish.message.topic.topic.size);
			topic_name[evt->param.publish.message.topic.topic.size] = '\0';

			subscriptions[i].callback(topic_name, payload, payload_length);
		}
	}
}

/// @brief Handler for asynchronous MQTT events
/// @param client the MQTT client instance
/// @param evt the MQTT event
static void mqtt_event_handler(struct mqtt_client *const client, const struct mqtt_evt *evt)
{
	switch (evt->type) {
	case MQTT_EVT_CONNACK:
		if (evt->result != 0) {
			LOG_ERR("MQTT Event Connect failed [%d]", evt->result);
			break;
		}
		on_mqtt_connect();
		break;

	case MQTT_EVT_DISCONNECT:
		on_mqtt_disconnect();
		break;

	case MQTT_EVT_PINGRESP:
		LOG_INF("PINGRESP packet");
		break;

	case MQTT_EVT_PUBACK:
		if (evt->result != 0) {
			LOG_ERR("MQTT PUBACK error [%d]", evt->result);
			break;
		}

		LOG_INF("PUBACK packet ID: %u", evt->param.puback.message_id);
		break;

	case MQTT_EVT_PUBREC:
		if (evt->result != 0) {
			LOG_ERR("MQTT PUBREC error [%d]", evt->result);
			break;
		}

		LOG_INF("PUBREC packet ID: %u", evt->param.pubrec.message_id);

		const struct mqtt_pubrel_param rel_param = {.message_id =
								    evt->param.pubrec.message_id};

		mqtt_publish_qos2_release(client, &rel_param);
		break;

	case MQTT_EVT_PUBREL:
		if (evt->result != 0) {
			LOG_ERR("MQTT PUBREL error [%d]", evt->result);
			break;
		}

		LOG_INF("PUBREL packet ID: %u", evt->param.pubrel.message_id);

		const struct mqtt_pubcomp_param rec_param = {.message_id =
								     evt->param.pubrel.message_id};

		mqtt_publish_qos2_complete(client, &rec_param);
		break;

	case MQTT_EVT_PUBCOMP:
		if (evt->result != 0) {
			LOG_ERR("MQTT PUBCOMP error %d", evt->result);
			break;
		}

		LOG_INF("PUBCOMP packet ID: %u", evt->param.pubcomp.message_id);
		break;

	case MQTT_EVT_SUBACK:
		if (evt->result == 0x80) {
			LOG_ERR("MQTT SUBACK error [%d]", evt->result);
			break;
		}

		LOG_INF("SUBACK packet ID: %d", evt->param.suback.message_id);
		break;

	case MQTT_EVT_PUBLISH:
		const struct mqtt_publish_param *p = &evt->param.publish;

		if (p->message.topic.qos == MQTT_QOS_1_AT_LEAST_ONCE) {
			const struct mqtt_puback_param ack_param = {.message_id = p->message_id};
			mqtt_publish_qos1_ack(client, &ack_param);
		} else if (p->message.topic.qos == MQTT_QOS_2_EXACTLY_ONCE) {
			const struct mqtt_pubrec_param rec_param = {.message_id = p->message_id};
			mqtt_publish_qos2_receive(client, &rec_param);
		}

		on_mqtt_publish(client, evt);

	default:
		break;
	}
}

/// @brief Poll the MQTT socket for received data
/// @param client the MQTT client instance
/// @param timeout the poll timeout in milliseconds
/// @return the number of events or negative errno on error
static int poll_mqtt_socket(struct mqtt_client *client, int timeout)
{
	// prepare the file descriptors for polling
	if (client->transport.type == MQTT_TRANSPORT_NON_SECURE) {
		fds[0].fd = client->transport.tcp.sock;
	}

	fds[0].events = ZSOCK_POLLIN;

	int events_count_or_error = zsock_poll(fds, 1, timeout);
	if (events_count_or_error < 0) {
		LOG_ERR("Socket poll error [%d]", events_count_or_error);
	}

	return events_count_or_error;
}

/// @brief Process incoming MQTT data and keep the connection alive*
/// @param client the MQTT client instance
/// @return 0 on success, negative errno on error
int app_mqtt_process(struct mqtt_client *client)
{
	int error;

	error = poll_mqtt_socket(client, mqtt_keepalive_time_left(client));
	if (error) {
		if (fds[0].revents & ZSOCK_POLLIN) {
			/* MQTT data received */
			error = mqtt_input(client);
			if (error != 0) {
				LOG_ERR("MQTT Input failed with error %d", error);
				return error;
			}
			/* Socket error */
			if (fds[0].revents & (ZSOCK_POLLHUP | ZSOCK_POLLERR)) {
				LOG_ERR("MQTT socket closed / error");
				return -ENOTCONN;
			}
		}
	} else {
		/* Socket poll timed out, time to call mqtt_live() */
		error = mqtt_live(client);
		if (error != 0) {
			LOG_ERR("MQTT Live failed [%d]", error);
			return error;
		}
	}

	return 0;
}

/// @brief Resolves the configured hostname and initializes the MQTT broker structure
/// @param  None
/// @return A code representing the result of the operation
static int broker_init(void)
{
	int error;
	struct addrinfo *broker_address;
	const struct addrinfo hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM};

	/* Resolve IP address of MQTT broker */
	error = getaddrinfo(CONFIG_MQTT_BROKER_HOSTNAME, NULL, &hints, &broker_address);
	if (error) {
		LOG_ERR("Broker hostname resolution failed with error %d", error);
		return -EIO;
	}
	if (broker_address == NULL) {
		LOG_ERR("Broker address not found");
		return -ENOENT;
	}

	/* Look for address of the broker. */
	while (broker_address != NULL) {
		/* IPv4 Address. */
		if (broker_address->ai_addrlen == sizeof(struct sockaddr_in)) {
			struct sockaddr_in *broker4 = ((struct sockaddr_in *)&broker);
			char ipv4_addr[NET_IPV4_ADDR_LEN];

			broker4->sin_addr.s_addr =
				((struct sockaddr_in *)broker_address->ai_addr)->sin_addr.s_addr;
			broker4->sin_family = AF_INET;
			broker4->sin_port = htons(CONFIG_MQTT_BROKER_PORT);

			inet_ntop(AF_INET, &broker4->sin_addr.s_addr, ipv4_addr, sizeof(ipv4_addr));
			LOG_INF("Broker IPv4 Address found: %s", (char *)(ipv4_addr));

			break;
		} else {
			LOG_ERR("ai_addrlen = %u should be %u or %u",
				(unsigned int)broker_address->ai_addrlen,
				(unsigned int)sizeof(struct sockaddr_in),
				(unsigned int)sizeof(struct sockaddr_in6));
		}

		broker_address = broker_address->ai_next;
	}

	/* Free the address. */
	freeaddrinfo(broker_address);

	return error;
}

int app_mqtt_init(struct mqtt_client *client, const uint8_t *client_id)
{
	int error;

	/* MQTT brokler initialization */
	error = broker_init();
	if (error) {
		LOG_ERR("Broker init failed with error %d", error);
		return error;
	}

	/* MQTT client configuration */
	mqtt_client_init(client);
	client->broker = &broker;
	client->evt_cb = mqtt_event_handler;
	client->client_id.utf8 = client_id;
	client->client_id.size = strlen(client->client_id.utf8);
	client->user_name = &mqtt_user_name;
	client->password = &mqtt_password;
	client->protocol_version = MQTT_VERSION_3_1_1;

	/* MQTT buffers configuration */
	client->rx_buf = rx_buffer;
	client->rx_buf_size = sizeof(rx_buffer);
	client->tx_buf = tx_buffer;
	client->tx_buf_size = sizeof(tx_buffer);

	return error;
}

void app_mqtt_connect(struct mqtt_client *client)
{
	int error = 0;

	mqtt_connected = false;

	/* Block until MQTT CONNACK event callback occurs */
	while (!mqtt_connected) {
		error = mqtt_connect(client);
		if (error) {
			LOG_ERR("MQTT Connect failed with error %d", error);
			k_sleep(K_SECONDS(CONFIG_MQTT_RECONNECT_WAIT_S));
			continue;
		}

		/* Poll MQTT socket for response */
		error = poll_mqtt_socket(client, CONFIG_MQTT_NET_POLL_INTERVAL_MS);
		if (error > 0) {
			mqtt_input(client);
		}

		if (!mqtt_connected) {
			mqtt_abort(client);
		}
	}
}

void app_mqtt_run(struct mqtt_client *client)
{
	/* Thread will primarily remain in this loop */
	while (mqtt_connected) {
		int error = app_mqtt_process(client);
		if (error) {
			LOG_ERR("MQTT Process failed with error %d", error);
			break;
		}
	}
	/* Gracefully close connection */
	mqtt_disconnect(client);
}

void app_mqtt_publish(struct mqtt_client *client, struct mqtt_topic *topic,
		      struct mqtt_binstr *payload)
{
	static uint16_t msg_id = 1;

	// create the publish parameters
	struct mqtt_publish_param param;
	param.message.topic = *topic;
	param.message.payload = *payload;
	param.message_id = msg_id++;
	param.dup_flag = 0;
	param.retain_flag = 0;

	int error = mqtt_publish(client, &param);
	if (error) {
		LOG_ERR("MQTT Publish failed with error %d", error);
		return;
	}

	LOG_INF("MQTT Publish succeeded");
}

void app_mqtt_subscribe(struct mqtt_client *client, struct mqtt_topic *topic, callback_t callback)
{
	static uint16_t msg_id = 1;

	// store the subscription in an empty slot
	int i;
	for (i = 0; i < CONFIG_MQTT_MAXIMUM_SUBSCRIPTIONS; i++) {
		if (subscriptions[i].callback == NULL) {
			subscriptions[i].topic.topic.utf8 = topic->topic.utf8;
			subscriptions[i].topic.topic.size = topic->topic.size;
			subscriptions[i].topic.qos = topic->qos;
			subscriptions[i].callback = callback;
			break;
		}
	}

	// check if the subscription was stored
	if (i == CONFIG_MQTT_MAXIMUM_SUBSCRIPTIONS) {
		LOG_ERR("MQTT Subscribe failed, no empty slot");
		return;
	}

	// create the subscribe parameters
	struct mqtt_subscription_list sub_list = {
		.list = topic, .list_count = 1, .message_id = msg_id++};

	// subscribe to the topic
	int error = mqtt_subscribe(client, &sub_list);
	if (error) {
		LOG_ERR("MQTT Subscribe failed with error %d", error);
		return;
	}

	LOG_INF("MQTT Subscribe to topic %s succeeded", topic->topic.utf8);
}

bool app_mqtt_connected(void)
{
	return mqtt_connected;
}