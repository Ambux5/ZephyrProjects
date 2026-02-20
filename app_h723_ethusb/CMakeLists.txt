cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(usb_webserver)

# Application sources
target_sources(app PRIVATE
    src/main.c
    src/usb_service.c
    src/web_server.c
)