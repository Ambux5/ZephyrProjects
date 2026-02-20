#include "web_server.h"
#include "led_service.h"

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(web_server, LOG_LEVEL_INF);

/* HTTP server listening port */
#define PORT 80

/* HTML response page with LED control buttons */
static const char *html_page =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n\r\n"
"<html><body>"
"<h1>Zephyr RTOS Demo</h1>"
"<form action=\"/on\" method=\"post\">"
"<button type=\"submit\">LED ON</button>"
"</form>"
"<br>"
"<form action=\"/off\" method=\"post\">"
"<button type=\"submit\">LED OFF</button>"
"</form>"
"</body></html>";

/* Web server thread - handles HTTP requests for LED control */
static void web_thread(void)
{
    /* Create TCP socket */
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    /* Configure server address and port */
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };

    /* Bind socket to address and port */
    bind(server, (struct sockaddr *)&addr, sizeof(addr));
    listen(server, 5);

    LOG_INF("Web server started on port 80");

    /* Accept incoming client connections */
    while (1) {

        int client = accept(server, NULL, NULL);
        if (client < 0) {
            continue;
        }

        /* Receive HTTP request from client */
        char buffer[512];
        int len = recv(client, buffer, sizeof(buffer)-1, 0);

        if (len > 0) {

            buffer[len] = 0;

            /* Check for LED ON request */
            if (strstr(buffer, "POST /on")) {
                led_cmd_t cmd = LED_CMD_ON;
                k_msgq_put(&led_msgq, &cmd, K_NO_WAIT);
            }

            /* Check for LED OFF request */
            if (strstr(buffer, "POST /off")) {
                led_cmd_t cmd = LED_CMD_OFF;
                k_msgq_put(&led_msgq, &cmd, K_NO_WAIT);
            }

            /* Send HTML response page to client */
            send(client, html_page, strlen(html_page), 0);
        }

        close(client);
    }
}

/* Define web server thread: 4KB stack, priority 7, starts automatically at boot */
K_THREAD_DEFINE(web_tid,
                4096,
                web_thread,
                NULL, NULL, NULL,
                7, 0, 0);

/* Web server initialization function (thread created via K_THREAD_DEFINE) */
void web_server_start(void)
{
    /* Automatic thread creation handled by K_THREAD_DEFINE macro */
}
