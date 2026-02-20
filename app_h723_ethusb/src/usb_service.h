/**
 * @file usb_service.h
 * @brief USB ECM (Ethernet Control Model) service initialization
 * 
 * Handles USB device initialization and network interface configuration
 * for USB-to-Ethernet connectivity.
 */

#pragma once

#include <zephyr/kernel.h>

/**
 * @brief Initialize USB device and configure network interface
 * 
 * Enables USB device, waits for network initialization, and configures
 * the USB ECM interface with IPv4 address 192.0.2.1/24.
 * 
 * @return 0 on success, negative error code on failure
 */
int usb_service_init(void);

/**
 * @brief Get the USB network interface
 * 
 * @return Pointer to USB network interface, or NULL if not found
 */
struct net_if *usb_service_get_interface(void);
