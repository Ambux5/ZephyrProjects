/**
 * @file web_server.c
 * @brief HTTP web server implementation
 * 
 * Simple HTTP/1.1 server that accepts connections on port 8080
 * and serves web pages from web_resources.h.
 */

#include "web_server.h"
#include "web_resources.h"
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

LOG_MODULE_REGISTER(web_server, LOG_LEVEL_INF);

/* HTTP server configuration */
#define HTTP_PORT 8080
#define LISTEN_BACKLOG 5
#define HTTP_THREAD_STACK_SIZE 2048
#define REQUEST_BUFFER_SIZE 512
#define RESPONSE_BUFFER_SIZE 1024  /* Minimal buffer for simple HTML pages */

/* Thread resources */
static K_THREAD_STACK_DEFINE(http_thread_stack, HTTP_THREAD_STACK_SIZE);
static struct k_thread http_thread_data;

/**
 * @brief Build HTTP response with proper Content-Length header
 * 
 * Creates a complete HTTP response with dynamically calculated Content-Length.
 * Buffer must be large enough for headers + content.
 * 
 * @param content HTML content body
 * @param response_buf Output buffer for complete response
 * @param buf_size Size of response buffer
 * @return Number of bytes written to buffer, or -1 on error
 */
static int build_http_response(const char *content, char *response_buf, size_t buf_size)
{
    if (!content || !response_buf) {
        return -1;
    }

    size_t content_len = strlen(content);
    
    /* Build HTTP headers with dynamic Content-Length */
    int header_len = snprintf(response_buf, buf_size,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        content_len);

    if (header_len < 0 || header_len >= (int)buf_size) {
        LOG_ERR("Failed to build HTTP headers");
        return -1;
    }

    /* Append content */
    if (header_len + content_len >= buf_size) {
        LOG_ERR("Response buffer too small (have %zu, need %zu)",
                buf_size, header_len + content_len);
        return -1;
    }

    memcpy(response_buf + header_len, content, content_len);
    return header_len + content_len;
}

/**
 * @brief Parse HTTP request path from request line
 * 
 * Extracts the path from HTTP request like "GET /index.html HTTP/1.1"
 * 
 * @param request Full HTTP request string
 * @param path Output buffer for parsed path
 * @param path_len Size of path buffer
 * @return 0 on success, -1 on error
 */
static int parse_http_request(const char *request, char *path, size_t path_len)
{
    /* Parse first line: "GET /path HTTP/1.1" */
    const char *start = strchr(request, ' ');
    if (!start) {
        return -1;
    }
    start++; /* Skip space after method */

    const char *end = strchr(start, ' ');
    if (!end) {
        return -1;
    }

    size_t len = end - start;
    if (len >= path_len) {
        return -1;
    }

    strncpy(path, start, len);
    path[len] = '\0';

    return 0;
}

/**
 * @brief HTTP server thread function
 * 
 * Continuously listens for incoming HTTP connections, parses requests,
 * and serves appropriate HTML pages based on the requested path.
 * 
 * @param arg1 Unused argument
 * @param arg2 Unused argument
 * @param arg3 Unused argument
 */
static void http_server_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    LOG_INF("HTTP server starting on port %d...", HTTP_PORT);

    /* Create TCP server socket */
    int server_socket = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0) {
        LOG_ERR("Failed to create socket: %d", errno);
        return;
    }

    /* Configure server address */
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(HTTP_PORT)
    };

    /* Bind socket to port */
    if (zsock_bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LOG_ERR("Failed to bind socket: %d", errno);
        zsock_close(server_socket);
        return;
    }

    /* Listen for incoming connections */
    if (zsock_listen(server_socket, LISTEN_BACKLOG) < 0) {
        LOG_ERR("Failed to listen: %d", errno);
        zsock_close(server_socket);
        return;
    }

    LOG_INF("HTTP server listening on port %d", HTTP_PORT);
    LOG_INF("Open http://192.0.2.1:%d in your browser", HTTP_PORT);

    /* Response buffer - static allocation for thread safety */
    static char response_buf[RESPONSE_BUFFER_SIZE];

    /* Accept and serve requests */
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        /* Accept incoming connection */
        int client_socket = zsock_accept(server_socket,
                                         (struct sockaddr *)&client_addr,
                                         &client_addr_len);
        if (client_socket < 0) {
            LOG_ERR("Failed to accept connection: %d", errno);
            continue;
        }

        LOG_INF("Client connected");

        /* Read HTTP request */
        char req_buf[REQUEST_BUFFER_SIZE];
        ssize_t req_len = zsock_recv(client_socket, req_buf, sizeof(req_buf) - 1, 0);
        
        if (req_len > 0) {
            req_buf[req_len] = '\0';
            LOG_DBG("HTTP Request:\n%s", req_buf);

            /* Parse requested path */
            char path[128] = {0};
            const char *content = html_not_found;

            if (parse_http_request(req_buf, path, sizeof(path)) == 0) {
                LOG_INF("Requested path: %s", path);

                /* Route requests */
                if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
                    content = html_index_page;
                    LOG_DBG("Serving index page");
                } else if (strcmp(path, "/status") == 0) {
                    content = html_status_page;
                    LOG_DBG("Serving status page");
                } else if (strcmp(path, "/info") == 0) {
                    content = html_info_page;
                    LOG_DBG("Serving info page");
                } else {
                    LOG_WRN("Path not found: %s", path);
                }
            } else {
                LOG_ERR("Failed to parse HTTP request");
            }

            /* Build complete HTTP response with proper Content-Length */
            int response_len = build_http_response(content, response_buf, sizeof(response_buf));
            
            if (response_len > 0) {
                /* Send HTTP response */
                ssize_t sent = zsock_send(client_socket, response_buf, response_len, 0);
                if (sent < 0) {
                    LOG_ERR("Failed to send response: %d", errno);
                } else {
                    LOG_INF("Sent %d bytes to client", sent);
                }
            } else {
                LOG_ERR("Failed to build HTTP response");
            }
        }

        zsock_close(client_socket);
    }

    zsock_close(server_socket);
}

int web_server_start(void)
{
    /* Create and start HTTP server thread */
    k_tid_t tid = k_thread_create(&http_thread_data,
                                   http_thread_stack,
                                   HTTP_THREAD_STACK_SIZE,
                                   http_server_thread,
                                   NULL, NULL, NULL,
                                   K_PRIO_PREEMPT(8),
                                   K_USER,
                                   K_NO_WAIT);

    if (tid == NULL) {
        LOG_ERR("Failed to create HTTP server thread");
        return -1;
    }

    return 0;
}
