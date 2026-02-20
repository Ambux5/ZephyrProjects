#include "led_service.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(led_service, LOG_LEVEL_INF);

/* LED0 GPIO pin configuration from device tree */
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* Message queue for LED commands: holds 10 messages, each 4 bytes aligned */
K_MSGQ_DEFINE(led_msgq, sizeof(led_cmd_t), 10, 4);

/* LED control thread - processes commands from web_server via message queue */
void led_thread(void)
{
    /* Initialize LED GPIO pin as output */
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

    led_cmd_t cmd;

    while (1) {
        /* Wait for LED command from message queue */
        if (k_msgq_get(&led_msgq, &cmd, K_FOREVER) == 0) {

            /* Turn LED on */
            if (cmd == LED_CMD_ON) {
                gpio_pin_set_dt(&led, 1);
                LOG_INF("LED ON");
            }

            /* Turn LED off */
            if (cmd == LED_CMD_OFF) {
                gpio_pin_set_dt(&led, 0);
                LOG_INF("LED OFF");
            }
        }
    }
}

/* Define LED thread: 1KB stack, priority 5, starts automatically at boot */
K_THREAD_DEFINE(led_tid, 1024,
                led_thread,
                NULL, NULL, NULL,
                5, 0, 0);
