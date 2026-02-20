cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nucleo_h723zg_app)

# Build main.c with HTTP server and LED control logic
target_sources(app PRIVATE
    src/main.c
    src/led_service.c
    src/web_server.c
)