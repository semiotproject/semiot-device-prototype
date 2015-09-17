# Arduino ESP8266 WiFI module SemIoT device prototype.

There are different ways to use this software:
+ directly flashing esp8266 without any additional Arduino boards
+ connecting esp8266 to some Arduino board via Serial port
and sending AT commands with manufacturer esp8266 firmware
(no need to re-flash esp8266)
+ using some Arduino board with any other WiFi-shield
+ `TODO`: compile for posix-compatible systems (for example, linux-based)

Most of the significant defines are in `./semiot-device/connections.h`


## Directly flashing esp8266 without any additional Arduino boards

Based on [esp8266 Arduino IDE libraries](https://github.com/esp8266/Arduino)
and microcoap implementation with SemIoT observe patches (#TODO: separate it)

### `TODO`:
+ fix sscanf missing
+ switch to [Bare Arduino Project](https://github.com/ladislas/Bare-Arduino-Project)
(implement platform.txt support for [Arduino-Makefile](https://github.com/sudar/Arduino-Makefile))
+ add new obsering resourse (ticker based)
+ contribute to used libs

## Connecting esp8266 to some Arduino board via Serial port

It's highly recommended to use esp8266 firmware higher than 0.9.4
with AT commands send higher than v0.2
to get udp working more or less well with manufacturer firmware.

You could find binary firmware for the esp8266 512KB flash memory version
in the /software/esp8266 project repo
with the PDF document containing appropriate AT commands description
from Espressif Systems IOT Team (v0.23).

You could easily flash the esp8266 from linux with the python esp8266tool like that:

```
esptool.py --port /dev/ttyACM0 --baud 115200 write_flash 0x000000 at023sdk101flash512k.bin
```

Note that after the flashing the firmware default baudrate will be 115200

If you have some problems with starting the flashing
you could try to reset esp8266 VCC pin to enter the flash writing mode.

If you don't have the USB TTL converter to connect the esp8266 directly to your machine
and have the arduino board,
try to use the built-in arduino usb-ttl converter similar to the scheme:

![ScreenShot](http://esp8266.ru/wp-content/uploads/esp8266-arduino_bb.jpg)

We are working on the libraries we using as well to provide better results.
We're providing ESP8266 WiFi library compatible with the Arduino WiFi API.

### Tested with Arduino MEGA 2560 and ESP8266 512K chip:
+ Arduino MEGA 2560
+ ESP8266 Device connected to Mega 2560 via Serial 3
+ DHT11 sensor connected to 2nd digital pin

![ScreenShot](https://dl.dropboxusercontent.com/u/39622126/Docs/semiot-shots/semiot-device_schem.png)
![ScreenShot](https://dl.dropboxusercontent.com/u/39622126/Docs/semiot-shots/semiot-device_bb.png)

## Using some Arduino board with any other WiFi-shield

`TODO`: not tested, should be similar to previous description

## Compile for posix-compatible systems (for example, linux-based)

`TODO`: implement it

# Additional notes

I recommend to use for testing [Wireshark](https://www.wireshark.org/) and
[libcoap](https://libcoap.net/):
```
coap-client -v 1 -p 5683 -m get coap://DEVICE_IP/.well-known/core
```
or [smcp](https://github.com/darconeous/smcp/):
```
smcpctl observe cp://DEVICE_IP:5683/dht_sensor/humidity
```

or and [Copper Firefox Plugin](https://addons.mozilla.org/En-us/firefox/addon/copper-270430/).
