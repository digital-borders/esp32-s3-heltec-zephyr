#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MAIN, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>

int main(void)
{
	LOG_INF("Starting main!");

	k_sleep(K_SECONDS(5));
}