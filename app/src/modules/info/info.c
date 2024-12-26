#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(INFO, LOG_LEVEL_INF);
#include <zephyr/kernel.h>
#include <zephyr/drivers/hwinfo.h>
#include "info_zbus.h"

#include <zephyr/sys/sys_heap.h>

// Debug Variables
extern struct k_heap _system_heap;
struct sys_memory_stats stats;
void rtos_hal_print_heap_info(void)
{
	sys_heap_runtime_stats_get(&_system_heap.heap, &stats);

	LOG_INF("\n");
	LOG_INF("INFO: Allocated Heap = %zu\n", stats.allocated_bytes);
	LOG_INF("INFO: Free Heap = %zu\n", stats.free_bytes);
	LOG_INF("INFO: Max Allocated Heap = %zu\n", stats.max_allocated_bytes);
	LOG_INF("\n");

	return;
}

/// @brief The info management thread
/// @param  None
/// @return The thread return code
int info_management_thread(void)
{
	LOG_INF("Initializing info module");
	if (!IS_ENABLED(CONFIG_HWINFO)) {
		LOG_ERR("HWINFO is not enabled! Cannot get device ID!");
		return -1;
	}
	uint8_t dev_uuid[6];
	int device_id_length = hwinfo_get_device_id(dev_uuid, sizeof(dev_uuid));
	if (device_id_length <= 0) {
		LOG_ERR("Failed to get device ID!");
		return -1;
	}
	// print the device id
	LOG_INF("Device ID: %02x:%02x:%02x:%02x:%02x:%02x", dev_uuid[0], dev_uuid[1], dev_uuid[2],
		dev_uuid[3], dev_uuid[4], dev_uuid[5]);
	// publish the device id
	struct info_device_message device_info;
	memcpy(device_info.device_uuid, dev_uuid, sizeof(dev_uuid));
	zbus_chan_pub(&info_device_channel, &device_info, K_FOREVER);

	while (1) {
		rtos_hal_print_heap_info();
		k_sleep(K_SECONDS(10));
	}

	// no more work to do
	return 0;
}
// Define the info management thread
K_THREAD_DEFINE(info_management_thread_id, 512, info_management_thread, NULL, NULL, NULL, 3, 0, 0);