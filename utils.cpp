#include "utils.h"

void prepareBusPin(int pin) {
  PORTD &= ~(1 << pin);
}

void prepareBusPins() {
  prepareBusPin(ATN_PIN);
  prepareBusPin(DATA_PIN);
  prepareBusPin(CLOCK_PIN);
}

void writePin(byte pin, boolean logicalState) {
  byte pinMask = (1 << pin);
  if (logicalState) {
    DDRD |= pinMask;
  } else {
    DDRD &= ~pinMask;
  }
}

byte readPin(byte pin) {
  byte pinMask = (1 << pin);
  return (PIND & pinMask) ? HIGH : LOW;
}

// DRIVE LED utils
void prepareDriveLedPin() {
  DDRB |= (1 << 5); // PIN13 as output
}

void setDriveLed(boolean turnOn) {
  if (turnOn) {
    PORTB |= (1 << 5);
  } else {
    PORTB &= ~(1 << 5);
  }
}
