/**
 * @file usb_service.c
 * @brief USB network service implementation
 *
 * Initializes the USB device network and configures the network interface
 * with a static IPv4 address for host communication.
 */

#include "usb_service.h"
#include <zephyr/usb/usb_device.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/dhcpv4_server.h>

LOG_MODULE_REGISTER(usb_service, LOG_LEVEL_INF);

/* Historically USB network interface was at index 2,
 * but this depends on board and configuration.
 */
#define USB_PREFERRED_IF_INDEX 2
#define USB_IF_SCAN_MAX_INDEX  10

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

    /* Give network stack time to initialize USB network interface */
    k_sleep(K_SECONDS(2));

    /* Try preferred index first */
    usb_iface = net_if_get_by_index(USB_PREFERRED_IF_INDEX);

    /* Fallback: scan available interfaces */
    if (!usb_iface) {
        for (int i = 1; i <= USB_IF_SCAN_MAX_INDEX; i++) {
            usb_iface = net_if_get_by_index(i);
            if (usb_iface) {
                LOG_WRN("Preferred USB interface index %d not found, using index %d",
                        USB_PREFERRED_IF_INDEX, i);
                break;
            }
        }
    }

    if (!usb_iface) {
        LOG_ERR("USB network interface not found");

        /* Debug: list available interfaces */
        LOG_INF("Available network interfaces:");
        for (int i = 1; i <= USB_IF_SCAN_MAX_INDEX; i++) {
            struct net_if *iface = net_if_get_by_index(i);
            if (iface) {
                LOG_INF("  Index %d: interface available", i);
            }
        }
        return -2;
    }

    LOG_INF("USB network interface selected");

    /* Configure static IPv4 address: 192.0.2.1/24 */
    struct in_addr addr;
    addr.s_addr = htonl((192 << 24) | (0 << 16) | (2 << 8) | 1);

    struct net_if_addr *ifaddr =
        net_if_ipv4_addr_add(usb_iface, &addr, NET_ADDR_MANUAL, 0);

    if (!ifaddr) {
        LOG_ERR("Failed to add IPv4 address to USB interface");
        return -3;
    }

    LOG_INF("IPv4 address 192.0.2.1/24 configured on USB interface");

    /* Start DHCP server for Windows auto-configuration */
    /* DHCP pool: 192.0.2.100 - 192.0.2.150 */
    struct in_addr dhcp_start;
    dhcp_start.s_addr = htonl((192 << 24) | (0 << 16) | (2 << 8) | 100);

    int dhcp_ret = net_dhcpv4_server_start(usb_iface, &dhcp_start);
    if (dhcp_ret < 0) {
        LOG_WRN("DHCP server failed to start: %d (manual configuration may be required on host)",
                dhcp_ret);
    } else {
        LOG_INF("DHCP server started at 192.0.2.1, pool 192.0.2.100-192.0.2.150");
    }

    return 0;
}

struct net_if *usb_service_get_interface(void)
{
    return usb_iface;
}