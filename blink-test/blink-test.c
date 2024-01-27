// ATMega328p @ 12MHz
#ifndef F_CPU
#define F_CPU 12000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
  // PB5 is the LED
  DDRB = 0b00100000;

  while (1) {
    PORTB = 0b00100000;
    _delay_ms(10000);
    PORTB = 0b00000000;
    _delay_ms(10000);
  }

  return 0;
}
