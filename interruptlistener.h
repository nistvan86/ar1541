#ifndef INTERRUPTLISTENER_H_
#define INTERRUPTLISTENER_H_

class InterruptListener {
public:
  virtual ~InterruptListener() {}

  virtual void atnLineChanged() = 0;
  virtual void clockLineChanged() = 0;
  virtual void dataLineChanged() = 0;
  virtual void resetLineChanged() = 0;
  virtual void timerInterrupt() = 0;
};

#endif /* INTERRUPTLISTENER_H_ */
