#ifndef IECDOSINTERPRETER_H_
#define IECDOSINTERPRETER_H_

#include <iectransreceiver.h>

#define DOS_STATE_ACCEPTING_COMMAND 0
#define DOS_STATE_BEGIN_OF_FILE_SENDING 1
#define DOS_STATE_SENDING_FILE 2

class IECTransreceiver;

class IECDOSInterpreter {
public:
  IECDOSInterpreter();

  void doProcessingOnMainLoop();
private:
  int state;
  int currentSendingFileByte;
  IECTransreceiver * transReceiver;

  void onBecameTalker();
  void onChannelOpenCommand(byte channel);
  void onChannelCloseCommand(byte channel);
  void onTransreceiverReset();

  virtual ~IECDOSInterpreter();
  void sendNextFileByte();
  friend class IECTransreceiver;
};

#endif /* IECDOSINTERPRETER_H_ */
