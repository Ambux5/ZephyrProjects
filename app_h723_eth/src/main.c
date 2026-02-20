#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "web_server.h"

LOG_MODULE_REGISTER(main_app, LOG_LEVEL_INF);

/* Main application entry point - monitors system health */
int main(void)
{
    LOG_INF("Zephyr RTOS LED Web Demo started");

    /* Main loop - periodically log system uptime */
    while (1) {
        k_sleep(K_SECONDS(10));
        LOG_INF("System alive, uptime: %u seconds",
                k_uptime_get() / 1000);
    }

    return 0;
}
