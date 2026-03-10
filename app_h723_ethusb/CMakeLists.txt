cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(usb_webserver)

# Read Zephyr version
file(READ "${ZEPHYR_BASE}/VERSION" ZEPHYR_VERSION_FILE)
string(REGEX MATCH "VERSION_MAJOR = ([0-9]+)" _ "${ZEPHYR_VERSION_FILE}")
set(ZEPHYR_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "VERSION_MINOR = ([0-9]+)" _ "${ZEPHYR_VERSION_FILE}")
set(ZEPHYR_MINOR ${CMAKE_MATCH_1})
string(REGEX MATCH "PATCHLEVEL = ([0-9]+)" _ "${ZEPHYR_VERSION_FILE}")
set(ZEPHYR_PATCH ${CMAKE_MATCH_1})
set(ZEPHYR_VERSION_STRING "${ZEPHYR_MAJOR}.${ZEPHYR_MINOR}.${ZEPHYR_PATCH}")

# Application sources
target_sources(app PRIVATE
    src/main.c
    src/usb_service.c
    src/web_server.c
)

# Pass version to compiler (without quotes for stringify macro)
target_compile_definitions(app PRIVATE
    ZEPHYR_VERSION_STRING=${ZEPHYR_MAJOR}.${ZEPHYR_MINOR}.${ZEPHYR_PATCH}
)