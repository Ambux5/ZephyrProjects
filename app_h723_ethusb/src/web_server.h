/**
 * @file web_server.h
 * @brief HTTP web server interface
 * 
 * Provides a simple HTTP server for web-based communication
 * over USB ECM interface.
 */

#pragma once

/**
 * @brief Start the HTTP web server
 * 
 * Initializes and starts the HTTP server thread on port 8080.
 * The server listens for incoming HTTP connections and serves
 * a simple web page.
 * 
 * @return 0 on success, negative error code on failure
 */
int web_server_start(void);
