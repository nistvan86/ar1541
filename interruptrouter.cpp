#include "interruptrouter.h"

#define ATN_PIN_MASK (1 << ATN_PIN)
#define RESET_PIN_MASK (1 << RESET_PIN)
#define ATN_AND_RESET_PINS_MASK (ATN_PIN_MASK | RESET_PIN_MASK)

volatile byte lastPINDCapture;

InterruptRouter::InterruptRouter() {
  initialized = false;
}

void InterruptRouter::init() {
  if (initialized) {
    return;
  }

  listener = NULL;
  prepareBusInterrupts();
  prepareTimer();
  initialized = true;
}

void InterruptRouter::setInterruptListener(InterruptListener * listener) {
  disableInterrupt(DATA_INTERRUPT);
  disableInterrupt(CLOCK_INTERRUPT);
  this->listener = listener;
}

void InterruptRouter::prepareTimer() {
  // Using timer1
  TCCR1B = _BV(WGM12) | _BV(CS11) | _BV(CS10); // Register B = PRESCALER64 (4us resolution) + CTC mode
  TIMSK1 = 0; // Disable timer interrupts
}

void InterruptRouter::startTimer(int microSeconds) {
  if (!initialized) {
    return;
  }

  int compareValue = microSeconds / 4;

  TCNT1 = 0; // Reset timer1
  OCR1A = compareValue; // (1/(16Mhz/64))*compareValue microseconds
  TIFR1 = _BV(OCF1A); // Reset interrupt flag on timer1
  TIMSK1 |= _BV(OCIE1A); // Enable timer1 compare interrupt
}

void InterruptRouter::stopTimer() {
  if (!initialized) {
    return;
  }

  TIMSK1 &= ~_BV(OCIE1A); // Disable interrupt for timer1
}

void InterruptRouter::prepareBusInterrupts() {
  // Enable the PIN interrupt on PORTD. This will capture the ATN and RESET changes. It's always on.
  lastPINDCapture = PIND;
  PCICR |= _BV(PCIE2); // Enable PCINT2 interrupt (PORTD pins)
  PCMSK2 = ATN_AND_RESET_PINS_MASK; // We are only interested about ATN and reset.

  // Prepare external interrupts on PIN2,3 (CLOCK, DATA). These are turned off by default.
  EICRA |= (1 << ISC00) | (1 << ISC10); // Any change on PIN2,3 generates an interrupt
  sei();
}

void InterruptRouter::enableInterrupt(byte interrupt) {
  if (!initialized) {
    return;
  }

  switch (interrupt) {
  case CLOCK_INTERRUPT:
    EIFR = (1 << INTF0);
    EIMSK |= (1 << INT0);
    break;
  case DATA_INTERRUPT:
    EIFR = (1 << INTF1);
    EIMSK |= (1 << INT1);
    break;
  }
}

void InterruptRouter::disableInterrupt(byte interrupt) {
  if (!initialized) {
    return;
  }

  switch (interrupt) {
  case CLOCK_INTERRUPT:
    EIMSK &= ~(1 << INT0);
    break;
  case DATA_INTERRUPT:
    EIMSK &= ~(1 << INT1);
    break;
  }
}

inline void clockInterrupt() {
  if (InterruptRouter::getInstance().listener != NULL) {
    InterruptRouter::getInstance().listener->clockLineChanged();
  }
}

ISR(INT0_vect) { // Clock interrupt function
  clockInterrupt();
}

inline void dataInterrupt() {
  if (InterruptRouter::getInstance().listener != NULL) {
    InterruptRouter::getInstance().listener->dataLineChanged();
  }
}

ISR(INT1_vect) { // Data interrupt function
  dataInterrupt();
}

inline void timerInterrupt() {
  if (InterruptRouter::getInstance().listener != NULL) {
    InterruptRouter::getInstance().listener->timerInterrupt();
  }
}

ISR(TIMER1_COMPA_vect) { // Called on Timer1 compare match interrupt
  timerInterrupt();
}

inline void resetInterrupt() {
  if (InterruptRouter::getInstance().listener != NULL) {
    InterruptRouter::getInstance().listener->resetLineChanged();
  }
}

inline void atnInterrupt() {
  if (InterruptRouter::getInstance().listener != NULL) {
    InterruptRouter::getInstance().listener->atnLineChanged();
  }
}

ISR(PCINT2_vect) { // Interrupt function called on PORTD PIN changes
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
