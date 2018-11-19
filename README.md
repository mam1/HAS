# HAS
## MQTT based Home Automation System
### Overview
* Mosquitto MQTT message broker broker implemented on a BeagleBbone Black
* ESP8266 based sensor and control modules
### BeagleBone Black rev C
BeagleBone Black rev C Running Debian jessi
#### Mosquitto
Install Eclipse Mosquitto (https://projects.eclipse.org/projects/technology.mosquitto)

Eclipse Mosquitto provides a lightweight server implementation of the MQTT protocol that is suitable for all situations from full power machines to embedded and low power machines. Sensors and actuators, which are often the sources and destinations of MQTT messages, can be very small and lacking in power. This also applies to the embedded machines to which they are connected, which is where Mosquitto could be run.

Typically, the current implementation of Mosquitto has an executable in the order of 120kB that consumes around 3MB RAM with 1000 clients connected. There have been reports of successful tests with 100,000 connected clients at modest message rates.

As well as accepting connections from MQTT client applications, Mosquitto has a bridge which allows it to connect to other MQTT servers, including other Mosquitto instances. This allows networks of MQTT servers to be constructed, passing MQTT messages from any location in the network to any other, depending on the configuration of the bridges.
#### Event scheduler
##### daemon
##### editor

### ESP8266 / nodeMCU
The ESP8266 is a low-cost Wi-Fi microchip with full TCP/IP stack and microcontroller capability produced by Shanghai-based Chinese manufacturer Espressif Systems. https://www.espressif.com/en/products/hardware/esp8266ex/overview

NodeMCU is an open source IoT platform. It includes firmware which runs on the ESP8266 Wi-Fi SoC from Espressif Systems, and hardware which is based on the ESP-12 module. The term "NodeMCU" by default refers to the firmware rather than the development kits. The firmware uses the Lua scripting language. https://nodemcu.readthedocs.io/en/master/

