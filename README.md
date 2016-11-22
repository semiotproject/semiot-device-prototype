# Arduino ESP8266 WiFI module SemIoT device prototypes.

`IN PROGRESS`

## How to build and flash

### set build env

+ get the [arduino for the esp8266 git version](https://github.com/esp8266/Arduino#using-git-version) to some folder: and checkout desired branch (2.3.0, for example) and download binary tools with the python2
+ specify path to gotten folder in Makefile for desired project (ESP_ROOT variable)
+ specify port if != /dev/ttyUSB0

### build and flash

+ run build bash script from project dir
+ answer yes after successful build to upload firmware

