#include "ar1541.h"

IECDOSInterpreter * dosInterpreter;

void setup() {
  Serial.begin(115200);

  prepareDriveLedPin();
  prepareBusPins();

  InterruptRouter::getInstance().init();
  dosInterpreter = new IECDOSInterpreter();
}

void loop() {
  dosInterpreter->doProcessingOnMainLoop();
}

