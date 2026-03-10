/**
 * @file main.c
 * @brief USB Microcontroller Web Configuration Demo
 * 
 * This application demonstrates a web server running over USB ECM
 * (Ethernet Control Model) on a microcontroller using Zephyr RTOS.
 * The device presents itself as an Ethernet device to the host.
 * Supports configuration of network settings and other parameters via web interface.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "usb_service.h"
#include "web_server.h"

LOG_MODULE_REGISTER(main_app, LOG_LEVEL_INF);

/**
 * @brief Main application entry point
 * 
 * Initializes USB ECM interface and starts the HTTP server.
 * The main thread monitors system health and logs periodic status updates.
 * 
 * @return 0 on success, non-zero on failure
 */
int main(void)
{
    LOG_INF("=== Zephyr USB ECM Web Server Application ===");

    /* Initialize USB ECM service */
    int ret = usb_service_init();
    if (ret < 0) {
        LOG_ERR("USB service initialization failed: %d", ret);
        return 1;
    }

    LOG_INF("USB ECM service initialized");

    /* Start HTTP web server */
    ret = web_server_start();
    if (ret < 0) {
        LOG_ERR("Web server startup failed: %d", ret);
        return 2;
    }

    LOG_INF("Web server started");
    LOG_INF("Ready for connections on http://192.0.2.1:8080");

    /* Main thread - monitor system health */
    while (1) {
        k_sleep(K_SECONDS(10));
        LOG_INF("System alive, uptime: %u seconds", k_uptime_get() / 1000);
    }

    return 0;
}
