# H723 USB ECM Web Server

This example demonstrates an HTTP web server running on an STM32H723 microcontroller over USB ECM (Ethernet Control Model) connection. The device appears as an Ethernet device to the Linux host, allowing network communication without traditional Ethernet hardware.

## Overview

The application provides:
- USB ECM device implementation (USB-to-Ethernet bridge)
- HTTP web server accessible at `http://192.0.2.1:8080`
- Automatic network interface configuration
- Clean, modular architecture with separate USB and web server components

## Prerequisites

### Linux Host System
- Linux kernel with USB ECM support (usually included by default)
- `ufw` firewall (optional, for security configuration)
- `net-tools` or `iproute2` for network verification

### Development Environment
- Zephyr SDK installed
- Python virtual environment setup

Create and activate virtual environment:

```console
python3 -m venv ~/zephyrproject/.venv
source ~/zephyrproject/.venv/bin/activate
```

## Hardware Setup

1. Connect STM32H723 to Linux host via USB cable (use USB connector on board)
2. Device will enumerate as USB ECM interface

## Linux Host Configuration

When the device is plugged in via USB, it will appear as a new network interface (typically `usb0`, `eth0`, or `enxXXXXXXXXXXXX`).

### Step 1: Find the USB Interface Name

```console
ip link
```

Look for the interface that appeared when you plugged in the device. It might be named like:
- `usb0`
- `eth0` (if it's the first Ethernet device)
- `enxXXXXXXXXXXXX` (where X are MAC address bytes)

Example output:
```
1: lo: <LOOPBACK,UP,LOWER_UP> ...
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> ...
3: enx00005e005301: <BROADCAST,MULTICAST> ...
```

### Step 2: Configure the USB Interface

If using `ufw` firewall (recommended):

```console
# Allow port 8080 for HTTP traffic
sudo ufw allow 8080/tcp

# Allow mDNS if using auto-discovery
sudo ufw allow 5353/udp
```

Configure the IP address on the USB interface (replace `enxXXXXXXXXXXXX` with your actual interface):

```console
# Bring up the interface
sudo ip link set enxXXXXXXXXXXXX up

# Add IPv4 address (device uses 192.0.2.1, host uses 192.0.2.2)
sudo ip addr add 192.0.2.2/24 dev enxXXXXXXXXXXXX
```

### Step 3: Verify Connection

```console
# Check interface is up
ip link show enxXXXXXXXXXXXX

# Check IP address configuration
ip addr show enxXXXXXXXXXXXX

# Ping the device
ping 192.0.2.1

# Check if port 8080 is responding
nc -zv 192.0.2.1 8080

# Or using curl
curl http://192.0.2.1:8080
```

Expected ping output:
```
PING 192.0.2.1 (192.0.2.1) 56(84) bytes of data.
64 bytes from 192.0.2.1: icmp_seq=1 ttl=64 time=1.23 ms
```

## Build and Flash

```console
cd ~/zephyrproject

# Clean and build for nucleo_h723zg
west build -t pristine
west build -p always -b nucleo_h723zg app_h723_ethusb

# Flash the board
west flash
```

If building from a different directory:

```console
west build -p always -b nucleo_h723zg /path/to/app_h723_ethusb
west flash
```

## Usage

### Monitor Serial Output

Open a terminal to monitor the application logs:

```console
# Find serial port (typically /dev/ttyACM0)
ls /dev/tty* | grep ACM

# Connect with minicom, picocom, or screen
sudo picocom -b 115200 /dev/ttyACM0

# Or using screen
sudo screen /dev/ttyACM0 115200

# View with dmesg (kernel messages)
dmesg -w
```

Expected serial output:
```log
*** Booting Zephyr OS build v4.x.x ***
[00:00:00.000,000] <inf> main_app: === Zephyr USB ECM Web Server Application ===
[00:00:00.050,000] <inf> usb_service: USB device initialized
[00:00:02.100,000] <inf> usb_service: USB ECM interface found at index 2
[00:00:02.150,000] <inf> usb_service: IPv4 address 192.0.2.1/24 configured on USB interface
[00:00:02.200,000] <inf> main_app: USB ECM service initialized
[00:00:02.250,000] <inf> web_server: HTTP server starting on port 8080...
[00:00:02.300,000] <inf> web_server: HTTP server listening on port 8080
[00:00:02.350,000] <inf> web_server: Open http://192.0.2.1:8080 in your browser
[00:00:02.400,000] <inf> main_app: Web server started
[00:00:02.450,000] <inf> main_app: Ready for connections on http://192.0.2.1:8080
[00:00:12.500,000] <inf> main_app: System alive, uptime: 10 seconds
```

### Access the Web Server

Once connected and configured:

```console
# Open in browser
curl http://192.0.2.1:8080

# Or use a web browser
firefox http://192.0.2.1:8080

# Or telnet
telnet 192.0.2.1 8080
```

## Troubleshooting

### Interface Not Appearing

If the USB interface doesn't appear after plugging in:

```console
# Check kernel messages
dmesg | tail -20

# Look for USB ECM related errors
dmesg | grep -i ecm

# Reload USB drivers
sudo modprobe -r cdc_ether
sudo modprobe cdc_ether

# Check USB devices
lsusb

# Detailed USB info
lsusb -v
```

### Cannot Ping Device

```console
# Verify interface configuration
ip addr show

# Check if interface is up
ip link show

# Check routing table
ip route show

# Try to restart interface
sudo ip link set enxXXXXXXXXXXXX down
sudo ip link set enxXXXXXXXXXXXX up

# Re-add address
sudo ip addr add 192.0.2.2/24 dev enxXXXXXXXXXXXX
```

### Port 8080 Not Responding

```console
# Check if address is correctly configured
ip addr show enxXXXXXXXXXXXX

# Try connecting with telnet
telnet 192.0.2.1 8080

# Check arp table
arp -a

# Try to force ARP resolution
arping -c 3 192.0.2.1

# Check kernel routing
ip route

# Check firewall rules
sudo ufw status
```

### Serial Port Permissions

If you get permission denied when accessing serial port:

```console
# Add your user to dialout group
sudo usermod -a -G dialout $USER

# Apply new group (log out and back in, or use)
newgrp dialout

# Alternatively, use sudo
sudo picocom -b 115200 /dev/ttyACM0
```

## Tested Hardware

| Device              | Status     |
| ------------------- | ---------- |
| nucleo_h723zg       | ✓ Tested   |

## Project Structure

```
app_h723_ethusb/
├── CMakeLists.txt              # Build configuration
├── prj.conf                    # Zephyr kernel configuration
├── prj-usb.conf               # USB-specific configuration
├── prj-eth.conf               # Ethernet configuration
├── prj-usb-cdc.conf           # USB CDC configuration
├── usb.overlay                # Device tree overlay for USB
├── README.md                  # This file
├── boards/
│   └── nucleo_h723zg.overlay  # Board-specific device tree
└── src/
    ├── main.c                 # Main application entry point and system monitoring
    ├── usb_service.c          # USB ECM initialization and configuration
    ├── usb_service.h          # USB service interface
    ├── web_server.c           # HTTP web server implementation
    └── web_server.h           # Web server interface
```

## Configuration

### Key Configuration Files

**prj.conf** - Primary Zephyr configuration:
- USB device support
- USB CDC ECM class
- Network stack (IPv4, TCP/UDP)
- Logging

**usb.overlay** - USB device tree customization:
- USB device string descriptors
- VID/PID configuration

**boards/nucleo_h723zg.overlay** - Board-specific device tree:
- USB pin mappings

## Architecture

The application uses a modular architecture for clean separation of concerns:

### USB Service (`usb_service.c/h`)
- Handles USB device initialization
- Configures USB ECM interface
- Sets static IP address (192.0.2.1/24)
- Manages network interface lifecycle

### Web Server (`web_server.c/h`)
- Implements HTTP/1.1 server
- Listens on port 8080
- Serves simple HTML page
- Independent of USB implementation

### Main Application (`main.c`)
- Initializes subsystems in order
- Monitors system health
- Logs periodic status updates
- Minimal business logic

## Security Considerations

This is a demonstration application. For production use:

1. **Enable Authentication**: Add HTTP authentication if exposing to untrusted networks
2. **Use HTTPS**: Implement TLS/SSL for encrypted communication
3. **Validate Input**: Sanitize any HTTP request parameters
4. **Rate Limiting**: Implement request rate limiting
5. **Firewall Rules**: Use `ufw` to restrict access to port 8080:
   ```console
   sudo ufw allow from 192.0.2.2 to 192.0.2.1 port 8080
   ```

## Common Issues and Solutions

### Issue: "Device not found" during flash

**Solution**: 
```console
# List available boards
west board list | grep h723

# Manual device selection
west flash --runner openocd --openocd-scripts /path/to/openocd
```

### Issue: Network initialization timeout

**Solution**: Check board has USB configured:
```console
# Verify USB is enabled in prj.conf
grep CONFIG_USB /home/ambu/zephyrproject/app_h723_ethusb/build/zephyr/.config
```

### Issue: High latency/packet loss

**Solution**: May be normal USB latency. If problematic:
1. Reduce USB polling interval
2. Check for other USB devices causing congestion
3. Try different USB port (prefer USB 2.0)

## Performance Notes

- HTTP response time: typically 1-5ms (USB ECM adds latency)
- Throughput: USB 2.0 up to 480 Mbps (ECM overhead ~10%)
- Concurrent connections: limited by memory (typically 2-5 simultaneous)

## References

- [Zephyr USB Device Support](https://docs.zephyrproject.org/latest/connectivity/usb/device.html)
- [Zephyr Networking](https://docs.zephyrproject.org/latest/connectivity/networking/)
- [Linux USB ECM Documentation](https://www.kernel.org/doc/html/latest/usb/gadget_configfs.html)
- [STM32H723 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00588877.pdf)

## Support

If you encounter issues:

1. Check serial output for error messages
2. Review troubleshooting section above
3. Check Zephyr logs with proper `CONFIG_LOG_LEVEL`
4. Use network sniffing tools to debug:
   ```console
   # Watch traffic on USB interface
   sudo tcpdump -i enxXXXXXXXXXXXX -n
   ```

