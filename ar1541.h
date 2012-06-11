#ifndef AR1541_H_
#define AR1541_H_
#include "Arduino.h"

#include <iectransreceiver.h>
#include <utils.h>
#include <constants.h>

#ifdef __cplusplus
extern "C" {
#endif
  void loop();
  void setup();
#ifdef __cplusplus
} // extern "C"
#endif

// These are called externally (interrupts or objects)
void eoiInterrupt();
void atnChangedInterrupt();
void clockChangedInterrupt();
void readyToSend();

void sendMiniProg();

#endif /* AR1541_H_ */
