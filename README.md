# Arduino ESP8266 WiFI module SemIoT device prototypes.

Based on [esp8266 Arduino IDE libraries](https://github.com/esp8266/Arduino)

## *How to flash ESP8266
If you don't have the USB TTL converter to connect the esp8266 directly to your machine
and have the arduino board,
try to use the built-in arduino usb-ttl converter similar to the scheme:

![ScreenShot](http://esp8266.ru/wp-content/uploads/esp8266-arduino_bb.jpg)

If you have some problems with starting the flashing
you could try to ground esp8266 RST pin to enter the flash writing mode.

## Supported devices

Two popular protocolos are currently supporting: RS-485 and impulse telemetry.
If you want to use RS-485-based device, you also should implement device-specific logic for esp8266-based modem.

### Impulse counter
Impulse counter basic connection idea:

![imp_bb](https://github.com/semiotproject/semiot-device-prototype/raw/master/doc/semiot_impulse_counter/impulse_connection_bb.png)

![imp_schema](https://github.com/semiotproject/semiot-device-prototype/raw/master/doc/semiot_impulse_counter/impulse_connection_schematic.png)


![impulse_connection](https://hsto.org/files/71e/713/cd1/71e713cd15dd4d4683105eddd48313fd.jpg)

esp8266 firmware: `./src/semiot_impulse_counter/semiot_impulse_counter.ino`

#### Supported impulse counter based devices

+ [Valtec VL-R-I water consumption meter](http://valtec.ru/catalog/pribory_ucheta/schetchiki_dlya_vody/vodoschetchik_universalnyj_s_impulsnym_vyhodom.html)

![VL-R-I](http://valtec.ru/image/goods/full//VLF-R-I.jpg)

SemIoT Gateway Device driver is also avaliable

+ [Incotex Mercury-201 electricity power consumption meter](http://www.incotexcom.ru/m201.htm)

![m201](http://www.incotexcom.ru/img/m201_2.jpg)

SemIoT Gateway Device driver is also avaliable

### RS-485 based devices

RS-485 counter basic connection idea via max485 interface:

![rs485_bb](https://github.com/semiotproject/semiot-device-prototype/raw/master/doc/semiot_rs485_device/rs485_connection_bb.png)
![rs485_schema](https://github.com/semiotproject/semiot-device-prototype/raw/master/doc/semiot_rs485_device/rs485_connection_schema.png)

#### Supported RS-485 counter based devices
+ [Energomera ce102r5-ak electricity power consumption meter](http://www.energomera.ru/ru/products/archive/ce102r5)

![ce102r5](http://www.energomera.ru/images/400x400/ce102r5/ce102_r5_main.jpg)

`IN PROGRESS`

+ [Teplovodokhran Pulsar water consumption meter](http://teplovodokhran.ru/products/schetchik-vody-kvartirnyy-pulsar-du-15.html)

![pulsar_w](http://teplovodokhran.ru.images.1c-bitrix-cdn.ru/upload/iblock/167/167e5dd8ccffa6ca82b63f58f82d8246.jpg?1436508211105364)

`COMING SOON`
