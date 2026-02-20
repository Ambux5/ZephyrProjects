#pragma once
#include <zephyr/kernel.h>

/* LED command enumeration for controlling LED state */
typedef enum {
    LED_CMD_ON,   /* Command to turn LED on */
    LED_CMD_OFF   /* Command to turn LED off */
} led_cmd_t;

/* Message queue for LED commands - shared with LED thread */
extern struct k_msgq led_msgq;
