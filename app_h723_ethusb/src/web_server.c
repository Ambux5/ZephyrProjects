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
#define RESPONSE_BUFFER_SIZE 32768  /* 32KB - for full HTML pages with extensive CSS */
#define MAX_FILE_PATH 256

/* Thread resources */
static K_THREAD_STACK_DEFINE(http_thread_stack, HTTP_THREAD_STACK_SIZE);
static struct k_thread http_thread_data;

/**
 * @brief Build HTTP response without Content-Length
 * 
 * HTTP response using Connection: close instead of Content-Length.
 * Client will read until connection closes, so no size issues.
 * 
 * @param content HTML content body
 * @param content_type HTTP Content-Type header
 * @param response_buf Output buffer for complete response
 * @param buf_size Size of response buffer
 * @return Number of bytes written to buffer, or -1 on error
 */
static int build_http_response(const char *content, const char *content_type,
                               char *response_buf, size_t buf_size)
{
    if (!content || !response_buf || !content_type) {
        return -1;
    }

    size_t content_len = strlen(content);
    
    /* Build HTTP headers WITHOUT Content-Length - use Connection: close instead */
    int header_len = snprintf(response_buf, buf_size,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        content_type);

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
 * @brief Handle static file requests (CSS, JS, etc)
 * 
 * @param path Requested file path
 * @param content_type Output content type
 * @param response_buf Output response buffer
 * @param buf_size Size of response buffer
 * @return Number of bytes in response, or -1 on error
 */
static int handle_static_file(const char *path, const char **content_type,
                              char *response_buf, size_t buf_size)
{
    /* Simple routing for Bootstrap files */
    const char *file_content = NULL;
    
    /* Bootstrap CSS - minimal inline version for embedded systems */
    if (strcmp(path, "/css/bootstrap.min.css") == 0) {
        *content_type = "text/css";
        file_content = "body{font-family:system-ui,-apple-system,'Segoe UI',Roboto,Arial,sans-serif}"
                      ".navbar{background:#0d6efd}.btn{padding:.375rem .75rem;border-radius:.25rem}"
                      ".card{border:1px solid #ddd;margin:1rem 0}.alert{padding:.75rem;margin:1rem 0}";
        
        size_t content_len = strlen(file_content);
        int header_len = snprintf(response_buf, buf_size,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Connection: close\r\n"
            "\r\n",
            *content_type);
        
        if (header_len > 0 && header_len + content_len < buf_size) {
            memcpy(response_buf + header_len, file_content, content_len);
            return header_len + content_len;
        }
    }
    
    /* Bootstrap JS - stub for now */
    if (strcmp(path, "/js/bootstrap.bundle.min.js") == 0) {
        *content_type = "application/javascript";
        file_content = "/* Bootstrap JS stub - not implemented in embedded version */";
        
        size_t content_len = strlen(file_content);
        int header_len = snprintf(response_buf, buf_size,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Connection: close\r\n"
            "\r\n",
            *content_type);
        
        if (header_len > 0 && header_len + content_len < buf_size) {
            memcpy(response_buf + header_len, file_content, content_len);
            return header_len + content_len;
        }
    }
    
    return -1;
}

/**
 * @brief Build HTTP 404 response
 */
static int build_404_response(char *response_buf, size_t buf_size)
{
    const char *content = html_not_found;
    size_t content_len = strlen(content);
    
    int header_len = snprintf(response_buf, buf_size,
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Connection: close\r\n"
        "\r\n",
        content_len);

    if (header_len < 0 || header_len >= (int)buf_size) {
        return -1;
    }

    if (header_len + content_len >= buf_size) {
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
            const char *content_type = "text/html; charset=UTF-8";

            if (parse_http_request(req_buf, path, sizeof(path)) == 0) {
                LOG_INF("Requested path: %s", path);

                /* Try static files first (CSS, JS, etc) */
                int static_response = handle_static_file(path, &content_type, response_buf, sizeof(response_buf));
                if (static_response > 0) {
                    /* Static file found and response built */
                    int total_sent = 0;
                    while (total_sent < static_response) {
                        ssize_t sent = zsock_send(client_socket, response_buf + total_sent,
                                                 static_response - total_sent, 0);
                        if (sent < 0) {
                            LOG_ERR("Failed to send response: %d", errno);
                            break;
                        } else if (sent == 0) {
                            break;
                        } else {
                            total_sent += sent;
                        }
                    }
                    zsock_close(client_socket);
                    continue;
                }

                /* Route HTML page requests */
                if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
                    content = html_index_page;
                    LOG_DBG("Serving index page");
                } else if (strcmp(path, "/status") == 0) {
                    content = html_status_page;
                    LOG_DBG("Serving status page");
                } else if (strcmp(path, "/config") == 0) {
                    content = html_config_page;
                    LOG_DBG("Serving config page");
                } else if (strcmp(path, "/info") == 0) {
                    content = html_info_page;
                    LOG_DBG("Serving info page");
                } else {
                    LOG_WRN("Path not found: %s", path);
                    /* Build 404 response */
                    int response_len = build_404_response(response_buf, sizeof(response_buf));
                    if (response_len > 0) {
                        int total_sent = 0;
                        while (total_sent < response_len) {
                            ssize_t sent = zsock_send(client_socket, response_buf + total_sent,
                                                     response_len - total_sent, 0);
                            if (sent < 0) {
                                LOG_ERR("Failed to send response: %d", errno);
                                break;
                            } else if (sent == 0) {
                                break;
                            } else {
                                total_sent += sent;
                            }
                        }
                    }
                    zsock_close(client_socket);
                    continue;
                }
            } else {
                LOG_ERR("Failed to parse HTTP request");
            }

            /* Build complete HTTP response with proper Content-Length */
            int response_len = build_http_response(content, content_type, response_buf, sizeof(response_buf));
            
            if (response_len > 0) {
                /* Send HTTP response - send all data, looping if needed */
                int total_sent = 0;
                while (total_sent < response_len) {
                    ssize_t sent = zsock_send(client_socket, response_buf + total_sent, 
                                             response_len - total_sent, 0);
                    if (sent < 0) {
                        LOG_ERR("Failed to send response: %d", errno);
                        break;
                    } else if (sent == 0) {
                        LOG_WRN("Socket write returned 0 bytes");
                        break;
                    } else {
                        total_sent += sent;
                    }
                }
                LOG_INF("Sent %d bytes to client (total)", total_sent);
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
