#ifndef IECTRANSRECEIVER_H_
#define IECTRANSRECEIVER_H_

#include "Arduino.h"
#include <utils.h>

#define DATA_BUFFER_SIZE 64

typedef void(*readyToSendFunc)();

class IECTransreceiver {
 public:
	  IECTransreceiver();
    void reset();
    void start();
    boolean sendByte(byte frame, boolean withEoi);
    void releaseClockLine();
    void atnLineChanged();
    void clockLineChanged();
    void dataLineChanged();
    void timerInterrupt();
    void setReadyToSendCallback(readyToSendFunc function);
    boolean isTalker();
    boolean isTransmitting();
 private:
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
    readyToSendFunc readyToSendCallback;

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
};

#endif /* IECTRANSRECEIVER_H_ */
