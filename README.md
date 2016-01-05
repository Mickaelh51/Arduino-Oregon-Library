# Arduino Oregon Library:
##Hardware:
* Arduino UNO
* 433Mhz receiver (connected on digital pin 2)
* Oregon sensors

## Features:
* handling THGR228N (Channel, ID, Temperature, Humidity, Battery level)
* handling THGN132N
* Checksum Control
* Reset Oregon IDs in EEPROM
* Learning mode (for add new Oregon sensors)

## Futures enhancements:
* OWL Electricty Meter
* WGR918 Annometer
* RGR918 Rain Guage
* UV138
* THGR918 Outside Temp-Hygro
* BTHR918
* Temp-Hygro-Baro

##WARNING
**The Unit Code (ID) is randomly set by the device upon insertion of the battery. It change (randomly) when you change battery**


## Example without MySensors:
Serial output:
```
test
```

## Example with MySensors library (www.mysensors.org):
Serial output:
```
test
```
## Result on OpenHAB controller (With MySensors)
![Logo](http://i.imgur.com/Tsne6yv.png)

## Helpers websites
- http://wmrx00.sourceforge.net/Arduino/OregonScientific-RF-Protocols.pdf
- http://www.connectingstuff.net/blog/encodage-protocoles-oregon-scientific-sur-arduino/ (FR)
- http://www.connectingstuff.net/blog/decodage-des-protocoles-oregon-scientific-sur-arduino-2/ (FR)
- https://github.com/erix/arduino/tree/master/weather
- http://www.instructables.com/id/Arduino-Wireless-Weather-Station-Web-Server/?ALLSTEPS
- https://raw.githubusercontent.com/onlinux/OWL-CMR180/master/arduino/oregon_owl.ino
- https://github.com/Cactusbone/ookDecoder/
- https://wiki.pilight.org/doku.php/oregon
