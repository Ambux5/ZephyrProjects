/**
 * @file web_resources.h
 * @brief Minimalist web server HTML resources
 * 
 * Lightweight HTML pages optimized for embedded systems.
 * No CSS styling to minimize memory footprint.
 */

#pragma once

/**
 * @brief Minimal HTML home page
 */
static const char *html_index_page =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "\r\n"
    "<!DOCTYPE html><html><head><title>H723</title></head><body>"
    "<h1>H723 Device</h1>"
    "<h2>Status</h2>"
    "<p>IP: 192.0.2.1:8080</p>"
    "<p>Connection: USB ECM</p>"
    "<p>Status: Online</p>"
    "<h2>Links</h2>"
    "<ul><li><a href=\"/status\">Status</a></li>"
    "<li><a href=\"/info\">Info</a></li></ul>"
    "</body></html>";

/**
 * @brief Status page
 */
static const char *html_status_page =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "\r\n"
    "<!DOCTYPE html><html><head><title>Status</title></head><body>"
    "<h1>System Status</h1>"
    "<pre>System: Running\nUSB: Connected\nNetwork: 192.0.2.1\nPort: 8080</pre>"
    "<p><a href=\"/\">Home</a></p>"
    "</body></html>";

/**
 * @brief Info page
 */
static const char *html_info_page =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "\r\n"
    "<!DOCTYPE html><html><head><title>Info</title></head><body>"
    "<h1>Device Info</h1>"
    "<pre>Device: STM32H723ZG\nBoard: NUCLEO\nRTOS: Zephyr</pre>"
    "<p><a href=\"/\">Home</a></p>"
    "</body></html>";

/**
 * @brief 404 Not Found page
 */
static const char *html_not_found =
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "\r\n"
    "<!DOCTYPE html><html><head><title>404</title></head><body>"
    "<h1>404 Not Found</h1>"
    "<p><a href=\"/\">Home</a></p>"
    "</body></html>";
