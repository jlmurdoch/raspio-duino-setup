# RasPiO Duino setup 

This is personal notes on how to get a [RasPiO Duino](https://rasp.io/duino/) that was released in 2015 up-and-running in 2024 with latest linux, avrdude+linuxgpio, etc.

The device is still handy for a few reasons:
- It plugs on top of the Raspberry Pi's GPIO (using only 26-pins)
- It logically works like an Arduino UNO
- External clock at 12MHz/3.3V (noting the UNO runs at 16MHz/5V)
- No flyleads between the devices
- One PSU for both devices
- Easy SPI comms - no bootloader needed
- Effectively supplies analogue inputs to a Raspberry Pi

Finally, this guide just uses avrdude, but there are Arduino IDE/CLI files board.txt, platform.txt and programmer.txt examples supplied [here](./arduino-files).

## Wiring / Pinouts
To access the RasPiO Duino, the following pins are used. Four are used for Avrdude linuxgpio programming and a further two can be used for a serial console:
| linuxgpio | Pi GPIO | Pi Pin | Pi Function |
|---------|---------|--------|-------------|
| RST | GPIO  8 | Pin 24 | SPI0 CE0  |
| SDI | GPIO  9 | Pin 21 | SPI0 MISO |
| SDO | GPIO 10 | Pin 19 | SPI0 MOSI |
| SCK | GPIO 11 | Pin 23 | SPI0 SCLK |
|  | GPIO 14 | Pin  8 | UART TX   |
|  | GPIO 15 | Pin 10 | UART RX   |

The linuxspi programmer doesn't work out-of-the-box. GPIO 8/SPI CE0 is hard wired to the RESET pin on the ATMega328P-PU and avrdude expects to use GPIO, which is then locked out by the spidev module / cited as being too slow for avrdude. The linuxgpio programmer should be used instead.

There is an LED on PB5 (5th bit when programming) that can be used for testing, which the blink-test program demonstrates.

## Installation
All that is needed is avrdude (for uploading) and the GCC AVR compiler suite:
```
sudo apt install avrdude gcc-avr
```

## Initial Testing For Connectivity
Run this to poll the device using the linuxgpio programmer, using the `avrdude-linuxgpio.conf`:
```
avrdude -p atmega328p -c linuxgpio -C +avrdude-linuxgpio.conf -v
avrdude: Version 7.1
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
If a test program is needed or one wishes to avoid using the Arduino IDE, in the blink-test directory is a compilation example that blinks the PB5 LED. 

Building the binary and hex output is as follows:
```
cd blink-test/
avr-gcc -Os -mmcu=atmega328p -c blink-test.c
avr-gcc -mmcu=atmega328p -o blink-test.elf blink-test.o 
avr-objcopy -O ihex blink-test.elf blink-test.hex
rm blink-test.o blink-test.elf
```

## Uploading To The Chip
To upload a hex file:
```
avrdude -p atmega328p -c linuxgpio -C +avrdude-linuxgpio.conf -U flash:w:blink-test/blink-test.hex
```

# Serial testing
If running on a Raspberry Pi, serial should be enabled, but **NOT** as a server, as the Pi will be a client.

Packages that can be used for serial comms:
- **GNU screen** - ability to detach / reattach
  - `screen /dev/ttyS0 9600`
- **minicom** - lots of options
  - `minicom -D /dev/ttyS0 -b 9600`
- **cu / opencu (call up)** - lightweight
  - `cu -s 9600 -l /dev/ttyS0`

# Chip fuse setup
Some notes on fuses:
- This only needs to be done on initialisation
- This was taken from a copy of avrsetup and converted to linuxgpio
- Extended fuse (efuse) is set as 0x07, but reported as 0xFF as only the first 3 bits can be toggled for brown-out detection
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
Here is how to play with an extracted ATMEGA328P-PU chip using a TL866xx and the minipro CLI tool. Using an ICSP interface with a RasPiO Duino still connected to a Raspberry Pi will probably result in magic smoke / dead electronics.  

Search for supported chip(s):
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
