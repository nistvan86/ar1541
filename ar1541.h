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
  void atexit( void ) { } // Required for Singleton classes for some reason to make GCC happy. :|
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* AR1541_H_ */
