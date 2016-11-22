MAIN_DIR ?= ..
SKETCH ?= $(MAIN_DIR)/semiot-device-dht-sleepy/semiot-device-dht-sleepy.ino
#SKETCH = semiot-device-dht-sleepy.ino
USER_LIBS ?= $(MAIN_DIR)/libraries
LIBS ?= $(ESP_LIBS)/Wire \
        $(ESP_LIBS)/EEPROM \
        $(ESP_LIBS)/ESP8266WiFi \
        $(ESP_LIBS)/ESP8266mDNS \
        $(ESP_LIBS)/Hash \
        $(USER_LIBS)/ArduinoJson \
        $(USER_LIBS)/DHT \
        $(USER_LIBS)/cantcoap \
        $(USER_LIBS)/minicoap

# Esp8266 Arduino git location
ESP_ROOT ?= $(MAIN_DIR)/esp8266

# Which variant to use from $(ESP_ROOT)/variants/
INCLUDE_VARIANT ?= d1_mini

# Board specific definitions
BOARD ?= d1_mini
FLASH_DEF ?= 4M3M
FLASH_MODE ?= dio
FLASH_SPEED ?= 40

# Upload parameters
#UPLOAD_SPEED ?= 460800
UPLOAD_PORT ?= /dev/ttyUSB0
UPLOAD_VERB ?= -v
UPLOAD_RESET ?= nodemcu

makeEspArduinoPath = $(ESP_ROOT)/../makeEspArduino
include $(MAIN_DIR)/makeEspArduino/makeEspArduino.mk
