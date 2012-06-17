#ifndef INTERRUPTROUTER_H_
#define INTERRUPTROUTER_H_

#include "Arduino.h"
#include <utils.h>
#include <interruptlistener.h>

#define CLOCK_INTERRUPT 0
#define DATA_INTERRUPT 1

class InterruptRouter {
public:
  static InterruptRouter& getInstance() {
    static InterruptRouter INSTANCE;

    return INSTANCE;
  }

  void init();
  void setInterruptListener(InterruptListener * listener);
  void startTimer(int microSeconds);
  void stopTimer();
  void enableInterrupt(byte interrupt);
  void disableInterrupt(byte interrupt);
private:
  InterruptListener * listener;
  bool initialized;

  InterruptRouter();
  InterruptRouter(InterruptRouter const&); // Don't implement
  void operator=(InterruptRouter const&); // Don't implement

  void prepareTimer();
  void prepareBusInterrupts();

  friend void clockInterrupt();
  friend void dataInterrupt();
  friend void atnInterrupt();
  friend void resetInterrupt();
  friend void timerInterrupt();
};

#endif /* INTERRUPTROUTER_H_ */
