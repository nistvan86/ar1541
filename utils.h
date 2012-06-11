#ifndef UTILS_H_
#define UTILS_H_

#include "Arduino.h"
#include <constants.h>

#define CLOCK_PIN 2
#define DATA_PIN 3
#define ATN_PIN 6
#define RESET_PIN 7

#define ATN_PIN_MASK (1 << ATN_PIN)
#define RESET_PIN_MASK (1 << RESET_PIN)

#define CLOCK_INTERRUPT 0
#define DATA_INTERRUPT 1

#define ATN_AND_RESET_PINS_MASK (ATN_PIN_MASK | RESET_PIN_MASK)

// PIN utils
void prepareBusPins();
void writePin(byte pin, boolean logicalState);
byte readPin(byte pin);

// DRIVE led utils
void prepareDriveLedPin();
void setDriveLed(boolean turnOn);

// TIMER utils
void prepareTimer();
void startTimer(int compare);
void stopTimer();
extern void timerInterrupt();

// PIN INTERRUPT utils
void prepareBusInterrupts();
void enableInterrupt(byte interrupt);
void disableInterrupt(byte interrupt);
extern void atnInterrupt();
extern void clockInterrupt();
extern void dataInterrupt();
extern void resetInterrupt();

#endif /* UTILS_H_ */
