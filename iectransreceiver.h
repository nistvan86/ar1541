#ifndef IECTRANSRECEIVER_H_
#define IECTRANSRECEIVER_H_

#include "Arduino.h"
#include <utils.h>
#include <interruptrouter.h>
#include <iecdosinterpreter.h>

#define DATA_BUFFER_SIZE 64

class IECDOSInterpreter;

class IECTransreceiver: public InterruptListener {
public:
  void reset();
  void start();
  boolean sendByte(byte frame, boolean withEoi);
  boolean isTalker();
  boolean isTransmitting();
private:
  IECTransreceiver(IECDOSInterpreter * dosInterpreter);
  IECTransreceiver(IECTransreceiver const&); // Don't implement
  void operator=(IECTransreceiver const&); // Don't implement

  IECDOSInterpreter * dosInterpreter;
  int state;
  boolean eoiReceived;
  boolean sendingWithEoi;
  byte currentDataByte;
  byte currentDataBit;
  byte dataBuffer[DATA_BUFFER_SIZE];
  byte dataBufferLength;

  boolean attention;
  boolean listener;
  boolean talker;
  byte channelToTalk;

  void goToIdleState();
  void beginAttention();
  void endAttention();
  void processTalkCommand();
  void processListenCommand();
  void processIncomingBit();
  void doTalkerTurnaround();
  void processUnlistenCommand();
  void processUntalkCommand();
  void confirmEoi();
  void startTransmission();
  void finishTransmission();
  void sendNextBit();

  void setState(int newState);
  void beginReceivingData();
  void resetDataBuffer();
  void printBuffer();
  void processData();
  void releaseClockLine();

  virtual void atnLineChanged();
  virtual void clockLineChanged();
  virtual void dataLineChanged();
  virtual void timerInterrupt();
  virtual void resetLineChanged();

  friend class IECDOSInterpreter;
};

#endif /* IECTRANSRECEIVER_H_ */
