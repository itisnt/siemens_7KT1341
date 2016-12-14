# Siemens 7KT1341

Extract data from a Siemens 7KT1341 Multifunctiional Counter over Modbus RTU

Should run on any "normal" unix box (tested on Odroid-C2, Beaglebone Black, Olimex A20)

## Getting Started

Add a USB-to-RS485 stick to your unix box. Make sure that the stick appears as a tty device like /dev/ttyUSB0.

### Prerequisites

* [libmodbus](http://libmodbus.org/) - A Modbus library for ....

Make sure that libmodbus ist installed. Either install it from http://libmodbus.org/ by hand or via your package manager.
Actual version is tested with libmodbus version 3.1.4

### Installing

Compile it with:

```
gcc siemens_7KT1341.c -o siemens_7KT1341 -I /usr/local/include/modbus -lmodbus
```
## Usage

```
./siemens_7KT1341 -d /dev/ttyUSB0
"28479";"236637";"1597";"10069";"238827";"627";"5672";"238329";"374";"22464";"1274";"50049";"44220";"53936";"10193323875";"12467271614"
```

## Help

```
./siemens_7KT1341 -h
------------------------------------------------------------------------------
Daten via Modbus aus einem Siemens 7KT1341 Stromzaehler auslesen
Zaehler muss via USB_to_RS485 Konverter am System haengen
------------------------------------------------------------------------------
Version 0.7
------------------------------------------------------------------------------
Usage  : ./siemens_7KT1341 -h | -d /dev/[serialport]
Example: ./siemens_7KT1341 -d /dev/ttyUSB0
------------------------------------------------------------------------------
Output:
 L1 Wirkleistung
 L1 Effektivespannung
 L1 Effektivestrom
 L2 Wirkleistung
 L2 Effektivespannung
 L2 Effektivestrom
 L3 Wirkleistung
 L3 Effektivespannung
 L3 Effektivestrom
 Temperatur
 Effektivstrom
 Frequenz
 L Summe Wirkleistung
 L Summe Scheinleistung
 Tarif 1 verbrauchte Wirkenergie
 Tarif 2 verbrauchte Wirkenergie

```

Sorry for some comments in german :-)

## Authors

* **Daniel B** - *Initial work* - [itisnt](https://github.com/itisnt)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Many thanks to Stéphane Raimbault for his libmodbus library

