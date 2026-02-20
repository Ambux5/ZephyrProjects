# Description

This example demonstrates an HTTP server that controls an onboard LED via web interface. The application uses Zephyr RTOS with Ethernet connectivity and DHCP to provide a responsive web-based LED control interface.

# Prerequisites

* Create virtual environment (venv is created only once):
  
  ```console
  python3 -m venv ~/zephyrproject/.venv
  ```

* Activate virtual environment (activated when you start working):
  
  ```console
  source ~/zephyrproject/.venv/bin/activate
  ```

# Build and flash

```console
cd ~/zephyrproject/zephyr
west build -p always -b <your-board-name> ../../zephyrtrain/24_Ethernet/http_server
west flash
```

Example:

```console
west build -t pristine
west build -p always -b nucleo_h723zg ../../zephyrtrain/24_Ethernet/http_server
west flash
```


# Tested board

| Device              |
| ------------------- |
| nucleo_h723zg       |

Many other boards are available in the boards directory, but not tested.

# Usage

1. Connect the board to Ethernet
2. Monitor serial output to get the assigned IP address (DHCP)
3. Open a browser and navigate to `http://<BOARD_IP>`
4. Use the web interface to control the LED

## API Endpoints

* **POST /on** - Turn LED ON
* **POST /off** - Turn LED OFF
* **GET /** - Display web interface

Example with curl:

```console
curl -X POST http://<BOARD_IP>/on
curl -X POST http://<BOARD_IP>/off
```

# Show the content via the debug serial port output

Open the debug serial port to show the Zephyr logging:

```log
*** Booting Zephyr OS build v4.x.x ***
[00:00:00.000,000] <inf> main_app: Zephyr RTOS LED Web Demo started
[00:00:00.100,000] <inf> web_server: Web server started on port 80
[00:00:10.000,000] <inf> main_app: System alive, uptime: 10 seconds
[00:00:15.200,000] <inf> web_server: Client connected
[00:00:15.205,000] <inf> led_service: LED ON
[00:00:20.100,000] <inf> web_server: Client connected
[00:00:20.105,000] <inf> led_service: LED OFF
```

# Configuration

Network settings in `prj.conf`:
* IPv4 enabled
* DHCP enabled
* Ethernet with STM32 HAL
* TCP sockets enabled

# Project Structure

```
.
├── boards/
│   └── nucleo_h723zg.overlay       # Device tree overlay (LED, Ethernet pins)
├── src/
│   ├── main.c              # Main application entry point
│   ├── web_server.c        # HTTP server implementation
│   ├── web_server.h        # HTTP server header
│   ├── led_service.c       # LED control thread
│   └── led_service.h       # LED service header
├── CMakeLists.txt          # Build configuration
├── prj.conf                # Zephyr configuration
└── README.md               # This file
```

## Board-specific Configuration

* `boards/nucleo_h723zg.overlay` - Device tree customization (LED pins, etc.)
