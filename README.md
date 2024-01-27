# RasPiO Duino setup 

This is notes on how to use a RasPiO Duino, an Arduino-compatible HAT for the Raspberry Pi. It runs an ATmega328p microcontroller at 12MHz with 32k memory). It is actually a clone of an Arduino UNO.

See the arduino-ide directory for notes on how to integrate into the Arduino IDE as part of the AVR package, although note that ARM64 packages are non-existant (Jan 2024), the fallback being a 32-bit install, or using Microsoft Visual Studio Code.

## Wiring / Pinouts
To access the RasPiO Duino for programming, SPI can be used with the linuxgpio programmer of avrdude:
- RST = GPIO  8, Pin 24, SPI0 CE0
- SDI = GPIO  9, Pin 21, SPI0 MISO
- SDO = GPIO 10, Pin 19, SPI0 MOSI
- SCK = GPIO 11, Pin 23, SPI0 SCLK

Serial access (optional) is as follows:
- TX = GPIO 14, Pin  8, UART TX
- RX = GPIO 15, Pin 10, UART RX

There is an LED on PB5 (5th bit when programming) that can be used for testing, which the blink-test program demonstrates.

## Installation
All you need is avrdude (for uploading) and the compiler suite:
```
sudo apt install avrdude gcc-avr
```

## Initial Testing For Connectivity
Run this to poll the device using the linuxgpio programmer:
```
vrdude: Version 7.1
         Copyright the AVRDUDE authors;
         see https://github.com/avrdudes/avrdude/blob/main/AUTHORS

         System wide configuration file is /etc/avrdude.conf
         User configuration file is /home/username/.avrduderc
         User configuration file does not exist or is not a regular file, skipping
         additional configuration file is avrdude-linuxgpio.conf

         Using Port                    : unknown
         Using Programmer              : linuxgpio
         AVR Part                      : ATmega328P
         Chip Erase delay              : 9000 us
         PAGEL                         : PD7
         BS2                           : PC2
         RESET disposition             : possible i/o
         RETRY pulse                   : SCK
         Serial program mode           : yes
         Parallel program mode         : yes
         Timeout                       : 200
         StabDelay                     : 100
         CmdexeDelay                   : 25
         SyncLoops                     : 32
         PollIndex                     : 3
         PollValue                     : 0x53
         Memory Detail                 :

                                           Block Poll               Page                       Polled
           Memory Type Alias    Mode Delay Size  Indx Paged  Size   Size #Pages MinW  MaxW   ReadBack
           ----------- -------- ---- ----- ----- ---- ------ ------ ---- ------ ----- ----- ---------
           eeprom                 65    20     4    0 no       1024    4      0  3600  3600 0xff 0xff
           flash                  65     6   128    0 yes     32768  128    256  4500  4500 0xff 0xff
           lfuse                   0     0     0    0 no          1    1      0  4500  4500 0x00 0x00
           hfuse                   0     0     0    0 no          1    1      0  4500  4500 0x00 0x00
           efuse                   0     0     0    0 no          1    1      0  4500  4500 0x00 0x00
           lock                    0     0     0    0 no          1    1      0  4500  4500 0x00 0x00
           signature               0     0     0    0 no          3    1      0     0     0 0x00 0x00
           calibration             0     0     0    0 no          1    1      0     0     0 0x00 0x00

         Programmer Type : linuxgpio
         Description     : Use the Linux sysfs interface to bitbang GPIO lines
         Pin assignment  : /sys/class/gpio/gpio{n}
           RESET   =  8
           SCK     =  11
           SDO     =  10
           SDI     =  9

avrdude: AVR device initialized and ready to accept instructions
avrdude: device signature = 0x1e950f (probably m328p)

avrdude done.  Thank you.
```

If the error "Can't export GPIO 8, already exported/busy?: Device or resource busy" appears, do this:
```
echo 8 > /sys/class/gpio/export
echo 8 > /sys/class/gpio/unexport
```

## Compiling An Example Program
If a test program is needed or you wish to avoid using the Arduino IDE, this is a compilation example, building the binary and hex output:
```
avr-gcc -Os -mmcu=atmega328p -c blink-test.c
avr-gcc -mmcu=atmega328p -o blink-test.elf blink-test.o 
avr-objcopy -O ihex blink-test.elf blink-test.hex
rm blink-test.o blink-test.elf
```

This code simply blinks the LED. A precompiled version is in the repository.

## Uploading To The Chip
To upload a hex file:
```
avrdude -p atmega328p -c linuxgpio -C +avrdude-linuxgpio.conf -U flash:w:blink-test/blink-test.hex
```

# Chip fuse setup
Some notes on fuses:
- This only needs to be done on initialisation
- This was taken from a copy of avrsetup and converted to linuxgpio
- Extended fuse (efuse) is set as 0x07, but reported as 0xFF as only the first 3 bits can be toggled for brown-out detection
)
- High fuse (hfuse) is set to 0xD9 giving the maximum of 2KB for a bootloader. This can be changed to 0xDA (0KB) if no bootloader is required
- This works for 12 MHz external crystal clock speed
```
avrdude -p atmega328p -c linuxgpio -C +avrdude-linuxgpio.conf -U lock:w:0x3F:m -U efuse:w:0x07:m -U lfuse:w:0xE7:m -U hfuse:w:0xD9:m
```

# Bootloader
If serial is required to program rather than SPI, a bootloader has to be installed.

The following should be suitable (untested!):
https://github.com/arduino/ArduinoCore-avr/blob/master/bootloaders/atmega/ATmegaBOOT_168_atmega328.hex

# Using a dedicated programmer
Here is how to play with the ATMEGA328P-PU chip using a TL866xx and the minipro CLI tool.

List for a supported chip:
```
$ minipro -L atmega328
Found TL866CS 03.2.86 (0x256)
Device code: 53122068
Serial code: 08FAE5934CC98F88D030EC32
ATMEGA328
ATMEGA328P
```

Check the ID to see if the chip is alive:
```
$ minipro -p ATMEGA328P -D
Found TL866CS 03.2.86 (0x256)
Device code: 53122068
Serial code: 08FAE5934CC98F88D030EC32
Chip ID: 0x1E950F  OK
```

Do a dump of the fuses, EEPROM (bootloader) and flash contents into files prefixed with dump.*:
```
minipro -p ATMEGA328P -r dump.bin
Found TL866CS 03.2.86 (0x256)
Device code: 53122068
Serial code: 08FAE5934CC98F88D030EC32
Chip ID: 0x1E950F  OK
Reading Code...  0.88Sec  OK
Reading Data...  0.04Sec  OK
Reading config... 0.00Sec  OK
```

