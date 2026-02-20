/**
 * @file usb_service.c
 * @brief USB ECM service implementation
 * 
 * Initializes USB device in ECM mode and configures the network interface
 * with a static IPv4 address for host communication.
 */

#include "usb_service.h"
#include <zephyr/usb/usb_device.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>

LOG_MODULE_REGISTER(usb_service, LOG_LEVEL_INF);

/* USB ECM interface is typically at index 2 */
#define USB_ECM_IF_INDEX 2

/* USB interface pointer cache */
static struct net_if *usb_iface = NULL;

int usb_service_init(void)
{
    /* Initialize USB device */
    if (usb_enable(NULL)) {
        LOG_ERR("USB initialization failed");
        return -1;
    }

    LOG_INF("USB device initialized");

    /* Give network stack time to initialize USB ECM interface */
    k_sleep(K_SECONDS(2));

    /* Get USB ECM interface */
    usb_iface = net_if_get_by_index(USB_ECM_IF_INDEX);
    if (!usb_iface) {
        LOG_ERR("USB interface not found at index %d", USB_ECM_IF_INDEX);
        
        /* Debug: list available interfaces */
        LOG_INF("Available network interfaces:");
        for (int i = 1; i <= 5; i++) {
            struct net_if *iface = net_if_get_by_index(i);
            if (iface) {
                LOG_INF("  Index %d: interface available", i);
            }
        }
        return -2;
    }

    LOG_INF("USB ECM interface found at index %d", USB_ECM_IF_INDEX);

    /* Configure static IPv4 address: 192.0.2.1/24 */
    struct in_addr addr;
    addr.s_addr = htonl((192 << 24) | (0 << 16) | (2 << 8) | 1);

    struct net_if_addr *ifaddr = net_if_ipv4_addr_add(usb_iface, &addr,
                                                       NET_ADDR_MANUAL, 0);
    if (!ifaddr) {
        LOG_ERR("Failed to add IPv4 address to USB interface");
        return -3;
    }

    LOG_INF("IPv4 address 192.0.2.1/24 configured on USB interface");

    return 0;
}

struct net_if *usb_service_get_interface(void)
{
    return usb_iface;
}
