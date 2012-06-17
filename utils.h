#ifndef UTILS_H_
#define UTILS_H_

#include "Arduino.h"
#include <constants.h>

// DON'T CHANGE!
#define CLOCK_PIN 2
#define DATA_PIN 3
#define ATN_PIN 6
#define RESET_PIN 7

// PIN utils
void prepareBusPins();
void writePin(byte pin, boolean logicalState);
byte readPin(byte pin);

// DRIVE led utils
void prepareDriveLedPin();
void setDriveLed(boolean turnOn);

#endif /* UTILS_H_ */
