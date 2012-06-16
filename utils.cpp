#include "utils.h"

// PIN utils
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

// TIMER utils
void prepareTimer() {
  // Using timer1
  TCCR1B = _BV(WGM12) | _BV(CS11) | _BV(CS10); // Register B = PRESCALER64 (4us resolution) + CTC mode
  TIMSK1 = 0; // Disable timer interrupts
}
void startTimer(int compare) {
  int compareValue = compare / 4;

  TCNT1 = 0; // Reset timer1
  OCR1A = compareValue; // (1/(16Mhz/64))*compareValue microseconds
  TIFR1 = _BV(OCF1A); // Reset interrupt flag on timer1
  TIMSK1 |= _BV(OCIE1A); // Enable timer1 compare interrupt
}

void stopTimer() {
  TIMSK1 &= ~_BV(OCIE1A); // Disable interrupt for timer1
}

ISR(TIMER1_COMPA_vect) { // Called on timer1 compare match
  timerInterrupt();
}

// PIN INTERRUPT utils
volatile byte lastPINDCapture;
void prepareBusInterrupts() {
  // Enable the PIN interrupt on PORTD. This will capture the ATN and RESET changes. It's always on.
  lastPINDCapture = PIND;
  PCICR |= _BV(PCIE2); // Enable PCINT2 interrupt (PORTD pins)
  PCMSK2 = ATN_AND_RESET_PINS_MASK; // We are only interested about ATN and reset.

  // Prepare external interrupts on PIN2,3 (CLOCK, DATA). These are turned off by default.
  EICRA |= (1 << ISC00) | (1 << ISC10); // Any change on PIN2,3 generates an interrupt
  sei();
}

void enableInterrupt(byte interrupt) {
  switch(interrupt) {
  case CLOCK_INTERRUPT:
    EIFR=(1<<INTF0);
    EIMSK|=(1<<INT0);
    break;
  case DATA_INTERRUPT:
    EIFR=(1<<INTF1);
    EIMSK|=(1<<INT1);
    break;
  }
}

void disableInterrupt(byte interrupt) {
  switch(interrupt) {
  case CLOCK_INTERRUPT:
    EIMSK&=~(1<<INT0);
    break;
  case DATA_INTERRUPT:
    EIMSK&=~(1<<INT1);
    break;
  }
}

ISR(PCINT2_vect) { // Called on PORTD PIN changes
  byte changes = (PIND ^ lastPINDCapture) & ATN_AND_RESET_PINS_MASK;
  lastPINDCapture = PIND;
  if (changes) {
    if ((PCMSK2 & RESET_PIN_MASK) && (changes & RESET_PIN_MASK)) {
      resetInterrupt();
    }
    if ((PCMSK2 & ATN_PIN_MASK) && (changes & ATN_PIN_MASK)) {
      atnInterrupt();
    }
  }
}

ISR(INT0_vect) { // Clock interrupt
  clockInterrupt();
}

ISR(INT1_vect) { // Clock interrupt
  dataInterrupt();
}
