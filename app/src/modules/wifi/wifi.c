#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(WIFI, LOG_LEVEL_ERR);

#include <zephyr/kernel.h>
#include <zephyr/net/wifi.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_event.h>
#include "wifi_zbus.h"

/// @brief A semaphore to indicate wifi connection status
static K_SEM_DEFINE(wifi_connected, 0, 1);

/// @brief A semaphore to indicate when an ipv4 address has been obtained
static K_SEM_DEFINE(ipv4_address_obtained, 0, 1);

/// @brief The network event callback for wifi
static struct net_mgmt_event_callback wifi_callback;

/// @brief The network event callback for ipv4
static struct net_mgmt_event_callback ipv4_callback;

/// @brief The wifi configuration message
static struct config_wifi_message wifi_configuration;

/// @brief Handles the wifi connect request result
/// @param event The event callback
static void handle_wifi_connect_result(struct net_mgmt_event_callback *callback)
{
	const struct wifi_status *status = (const struct wifi_status *)callback->info;

	if (status->conn_status != WIFI_STATUS_CONN_SUCCESS) {
		LOG_ERR("Connection request failed (%d)", status->status);
	} else {
		LOG_INF("Connected!");
		k_sem_give(&wifi_connected);
	}
}

/// @brief Handles the wifi disconnect request result
/// @param event The event callback
static void handle_wifi_disconnect_result(struct net_mgmt_event_callback *callback)
{
	const struct wifi_status *status = (const struct wifi_status *)callback->info;

	if (status->conn_status != WIFI_STATUS_CONN_SUCCESS) {
		LOG_INF("Disconnection request (%d)", status->status);
	} else {
		LOG_INF("Disconnected");
	}
}

/// @brief Handles the ipv4 address result
/// @param iface The network interface
static void handle_ipv4_result(struct net_if *iface)
{
	int i = 0;

	for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {

		char buf[NET_IPV4_ADDR_LEN];

		if (iface->config.ip.ipv4->unicast[i].ipv4.addr_type != NET_ADDR_DHCP) {
			continue;
		}

		LOG_INF("IPv4 address: %s",
			net_addr_ntop(AF_INET,
				      &iface->config.ip.ipv4->unicast[i].ipv4.address.in_addr, buf,
				      sizeof(buf)));
		LOG_INF("Subnet: %s",
			net_addr_ntop(AF_INET, &iface->config.ip.ipv4->unicast->netmask, buf,
				      sizeof(buf)));
		LOG_INF("Router: %s",
			net_addr_ntop(AF_INET, &iface->config.ip.ipv4->gw, buf, sizeof(buf)));

		k_sem_give(&ipv4_address_obtained);
	}
}

/// @brief The network event handler
/// @param callback The event callback
/// @param event The event type
/// @param interface The network interface
static void network_mgmt_event_handler(struct net_mgmt_event_callback *callback, uint32_t event,
				       struct net_if *interface)
{
	switch (event) {
	case NET_EVENT_WIFI_CONNECT_RESULT:
		handle_wifi_connect_result(callback);
		break;

	case NET_EVENT_WIFI_DISCONNECT_RESULT:
		handle_wifi_disconnect_result(callback);
		break;

	case NET_EVENT_IPV4_ADDR_ADD:
		handle_ipv4_result(interface);
		break;

	default:
		break;
	}
}

/// @brief Get the wifi connection status
/// @param None
/// @return True if connected, false otherwise
static bool wifi_get_status(void)
{
	struct net_if *iface = net_if_get_default();
	struct wifi_iface_status status = {0};

	if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status,
		     sizeof(struct wifi_iface_status))) {
		LOG_ERR("Connection status request failed");
		return false;
	}

	if (status.state >= WIFI_STATE_ASSOCIATED) {
		LOG_DBG("Actually connected to SSID: %s with RSSI: %d", status.ssid, status.rssi);
		return true;
	}

	return false;
}

/// @brief Start a connect request using the default network interface
/// @param ssid the pointer to the SSID string
/// @param psk the pointer to the PSK string
/// @return True if connection request was successful, false otherwise
static bool wifi_connect(char *ssid, char *psk)
{
	struct net_if *interface = net_if_get_default();
	struct wifi_connect_req_params wifi_params = {0};

	// set connection parameters
	wifi_params.ssid = ssid;
	wifi_params.psk = psk;
	wifi_params.ssid_length = strlen(ssid);
	wifi_params.psk_length = strlen(psk);
	wifi_params.channel = WIFI_CHANNEL_ANY;
	wifi_params.security = WIFI_SECURITY_TYPE_PSK;
	wifi_params.band = WIFI_FREQ_BAND_2_4_GHZ;
	wifi_params.mfp = WIFI_MFP_OPTIONAL;

	LOG_INF("Connecting to SSID: %s", wifi_params.ssid);

	if (net_mgmt(NET_REQUEST_WIFI_CONNECT, interface, &wifi_params,
		     sizeof(struct wifi_connect_req_params))) {
		LOG_ERR("Connection request failed");
		return false;
	}

	// wait for connection for a maximum of 20 seconds
	if (k_sem_take(&wifi_connected, K_SECONDS(20))) {
		LOG_ERR("Connection timed out");
		return false;
	}

	// connection successful
	k_sem_reset(&wifi_connected);
	return true;
}

/// @brief Start a disconnect request using the default network interface
/// @param None
static void wifi_disconnect(void)
{
	struct net_if *interface = net_if_get_default();

	if (net_mgmt(NET_REQUEST_WIFI_DISCONNECT, interface, NULL, 0)) {
		LOG_ERR("Disconnect request failed");
	}
}

/// @brief Update the wifi configuration
/// @return True if the configuration was updated, false otherwise
static bool wifi_update_config()
{
	bool updated = false;
	const struct zbus_channel *channel;

	updated = (zbus_sub_wait(&wifi_thread_subscriber, &channel, K_NO_WAIT) == 0);

	if (updated) {
		// configuration has potentially changed! Modify the wifi configuration
		zbus_chan_read(&config_wifi_channel, &wifi_configuration, K_FOREVER);
	}

	return updated;
}

/// @brief Wait until an ipv4 address is obtained
/// @param None
/// @return True if an address was obtained, false in case of timeout
static bool ipv4_obtain_address(void)
{
	// wait for an address for a maximum of 20 seconds
	if (k_sem_take(&ipv4_address_obtained, K_SECONDS(20))) {
		LOG_ERR("Get Ipv4 timed out");
		return false;
	}

	// address obtained
	k_sem_reset(&ipv4_address_obtained);
	return true;
}

/// @brief Notify the wifi status to the wifi status channel
/// @param connected True if connected, false otherwise
static void notify_wifi_status(bool connected)
{
	struct status_wifi_message wifi_status = {.connected = connected};

	LOG_INF("Wifi status: %d", wifi_status.connected);
	LOG_DBG("Publishing wifi status");
	zbus_chan_pub(&status_wifi_channel, &wifi_status, K_NO_WAIT); // todo
}

/// @brief The wifi management thread
/// @param  None
/// @return The thread return code
int wifi_management_thread(void)
{
	// indicates a connection failure
	bool failure = false;

	LOG_INF("Initializing wifi module");

	/* Sleep 1 seconds to allow initialization of wifi driver. */
	k_sleep(K_SECONDS(1));

	// add the handler for wifi events
	net_mgmt_init_event_callback(&wifi_callback, network_mgmt_event_handler,
				     NET_EVENT_WIFI_CONNECT_RESULT |
					     NET_EVENT_WIFI_DISCONNECT_RESULT);
	net_mgmt_add_event_callback(&wifi_callback);

	// add the handler for ipv4 events
	net_mgmt_init_event_callback(&ipv4_callback, network_mgmt_event_handler,
				     NET_EVENT_IPV4_ADDR_ADD);
	net_mgmt_add_event_callback(&ipv4_callback);

	// wifi simple connection manager
	while (1) {
		notify_wifi_status(false);

		if (failure) {
			// connection failed. Wait for 5 seconds before retrying
			LOG_INF("Connection failed. Retrying in 5 seconds");
			k_sleep(K_SECONDS(5));
		}

		// get the wifi configuration
		wifi_update_config();

		// if ssid or psk is empty
		if (strlen(wifi_configuration.ssid) == 0 || strlen(wifi_configuration.psk) == 0) {
			LOG_INF("Invalid configuration. Retrying in 5 seconds");
			k_sleep(K_SECONDS(5));
			continue;
		}

		// start clear by disconnecting
		wifi_disconnect();

		// try to connect to wifi
		if (wifi_connect(wifi_configuration.ssid, wifi_configuration.psk) == false) {
			failure = true;
			continue;
		}

		// try to obtain an ipv4 address
		if (ipv4_obtain_address() == false) {
			failure = true;
			continue;
		}

		// connection successful
		failure = false;
		notify_wifi_status(true);

		// check the wifi status
		while (1) {
			if (wifi_get_status() == false) {
				// connection failed somehow
				break;
			}

			// check for configuration changes
			if (wifi_update_config()) {
				// configuration has changed. Repeat connection process
				break;
			}

			// wait for 2 seconds before checking again
			k_sleep(K_SECONDS(2));
		}
	}
}

// Define the wifi management thread
K_THREAD_DEFINE(wifi_management_thread_id, 2048, wifi_management_thread, NULL, NULL, NULL, 3, 0, 0);