# HAS
## MQTT based Home Automation System
### Overview
* Mosquitto MQTT message broker broker implemented on a BeagleBbone Black
* Schedule editor and control daemon written in C running on the BeagleBone
* ESP8266 based sensor and control modules
### BeagleBone Black rev C 
BeagleBone Black is a low-cost, community-supported development platform for developers and hobbyists. It will boot Linux in under 10 seconds. http://beagleboard.org/black

Hardware
* Processor: AM335x 1GHz ARM® Cortex-A8
* 512MB DDR3 RAM
* 4GB 8-bit eMMC on-board flash storage
* 3D graphics accelerator
* NEON floating-point accelerator
* 2x PRU 32-bit microcontrollers

Connectivity
* USB client for power & communications
* USB host
* Ethernet
* HDMI

#### Mosquitto
Install Eclipse Mosquitto (https://projects.eclipse.org/projects/technology.mosquitto) https://www.evernote.com/shard/s416/sh/d6f2812d-600e-4d2f-9201-cc0fb1577bcb/b69ec1afc266e85f5f0e6dfe05b6e077

Eclipse Mosquitto provides a lightweight server implementation of the MQTT protocol that is suitable for all situations from full power machines to embedded and low power machines. Sensors and actuators, which are often the sources and destinations of MQTT messages, can be very small and lacking in power. This also applies to the embedded machines to which they are connected, which is where Mosquitto could be run.

Typically, the current implementation of Mosquitto has an executable in the order of 120kB that consumes around 3MB RAM with 1000 clients connected. There have been reports of successful tests with 100,000 connected clients at modest message rates.

As well as accepting connections from MQTT client applications, Mosquitto has a bridge which allows it to connect to other MQTT servers, including other Mosquitto instances. This allows networks of MQTT servers to be constructed, passing MQTT messages from any location in the network to any other, depending on the configuration of the bridges.
#### Event scheduler
##### daemon
##### editor

### ESP8266 / nodeMCU
The ESP8266 is a low-cost Wi-Fi microchip with full TCP/IP stack and microcontroller capability produced by Shanghai-based Chinese manufacturer Espressif Systems. https://www.espressif.com/en/products/hardware/esp8266ex/overview

NodeMCU is an open source IoT platform. It includes firmware which runs on the ESP8266 Wi-Fi SoC from Espressif Systems, and hardware which is based on the ESP-12 module. The term "NodeMCU" by default refers to the firmware rather than the development kits. The firmware uses the Lua scripting language. https://nodemcu.readthedocs.io/en/master/

