#include "iectransreceiver.h"

// Idle states
#define STATE_RESET -1
#define STATE_IDLE 0
// Listener states
#define STATE_WAIT_FOR_TALKER 1
#define STATE_READY_FOR_DATA 2
#define STATE_DATA_RECEIVED 3
// Talker states
#define STATE_BECOMING_TALKER 4
#define STATE_TALKER_IDLE 5
#define STATE_TALKER_WAIT_FOR_DATA_ACCEPT 6
#define STATE_TALKER_WAIT_FOR_EOI_HANDSHAKE 7
#define STATE_TALKER_WRITING_DATA_BITS 8
#define STATE_TALKER_WAIT_FOR_FRAME_ACKNOWLEDGEMENT 9

IECTransreceiver::IECTransreceiver() {
  readyToSendCallback = NULL;
  reset();
}

void IECTransreceiver::reset() {
  if (state == STATE_RESET)
    return;

  Serial.println("R");
  setState(STATE_RESET);
  stopTimer();
  attention = false;
  listener = false;
  talker = false;
  eoiReceived = false;
  disableInterrupt(DATA_INTERRUPT);
  enableInterrupt(CLOCK_INTERRUPT);
  writePin(DATA_PIN, false); // Release data pin if it's hold down
  writePin(CLOCK_PIN, false); // Release the data line
}

void IECTransreceiver::start() {
  if (state == STATE_RESET) {
    Serial.print(">");
    goToIdleState();
  }
}

boolean IECTransreceiver::sendByte(byte frame, boolean withEoi) {
  if (!talker || state == STATE_RESET || state >= STATE_TALKER_WAIT_FOR_DATA_ACCEPT) {
    return false;
  }

  currentDataByte = frame;
  currentDataBit = 0;
  sendingWithEoi = withEoi;

  setState(STATE_TALKER_WAIT_FOR_DATA_ACCEPT);
  enableInterrupt(DATA_INTERRUPT);
  writePin(CLOCK_PIN, false); // release the clock to flag that we are ready to send data

  return true;
}

void IECTransreceiver::releaseClockLine() {
  writePin(CLOCK_PIN, false);
}

void IECTransreceiver::atnLineChanged() {
  if (state == STATE_RESET)
    return;

  byte atn = readPin(ATN_PIN);
  if (atn == LOW) {
    beginAttention();
  } else { // ATN is HIGH
    endAttention();
  }
}

void IECTransreceiver::clockLineChanged() {
  if (state == STATE_RESET || (!listener && !attention && state != STATE_BECOMING_TALKER)) {
    return;
  }

  byte clock = readPin(CLOCK_PIN);
  if (clock == HIGH) {
    if (state == STATE_WAIT_FOR_TALKER) { // Talker signaled that it's ready to send data
      // TODO: Hold can be inserted here to prepare for the start of transmission
      beginReceivingData();
    } else if (state == STATE_READY_FOR_DATA) { // Talker sent us a bit
      processIncomingBit();
    } else if (state == STATE_DATA_RECEIVED) { // Talker waits for us to signal that it can send the next frame

      if (dataBufferLength == DATA_BUFFER_SIZE - 1) { // Handle overflow if required
        if (!attention) {
          processData();
        } else {
          Serial.print('!');
        }
      }

      beginReceivingData();
    } else if (state == STATE_BECOMING_TALKER) { // Turnaround. Previous talker releases the clock line and holds the data line as a listener.
      doTalkerTurnaround();
    }
  } else { // CLOCK is LOW
    if (state == STATE_READY_FOR_DATA) { // Talker started to prepare bit on the line
      stopTimer(); // Stop the EOI timer.
    }
  }
}

void IECTransreceiver::dataLineChanged() {
  if (state == STATE_RESET || !talker)
    return;

  byte data = readPin(DATA_PIN);
  if (data == HIGH) {
    if (state == STATE_TALKER_WAIT_FOR_DATA_ACCEPT) {
      if (sendingWithEoi) {
        setState(STATE_TALKER_WAIT_FOR_EOI_HANDSHAKE);
      } else {
        startTransmission();
      }
    } else if (state == STATE_TALKER_WAIT_FOR_EOI_HANDSHAKE) {
      startTransmission();
    }
  } else { // DATA is LOW
    if (state == STATE_TALKER_WAIT_FOR_EOI_HANDSHAKE) {
      // Stay in this state, HIGH edge will proceed.
    } else if (state == STATE_TALKER_WAIT_FOR_FRAME_ACKNOWLEDGEMENT) {
      // Listener acknowledged the frame
      finishTransmission();
    }
  }
}

void IECTransreceiver::timerInterrupt() {
  if (listener && state == STATE_READY_FOR_DATA) { // EOI signaled
    confirmEoi();
  } else if (talker && state == STATE_TALKER_WRITING_DATA_BITS) {
    stopTimer();
    sendNextBit();
  }
}

void IECTransreceiver::setReadyToSendCallback(readyToSendFunc function) {
  readyToSendCallback = function;
}

boolean IECTransreceiver::isTalker() {
  return state >= STATE_TALKER_IDLE;
}

boolean IECTransreceiver::isTransmitting() {
  return state >= STATE_TALKER_WAIT_FOR_DATA_ACCEPT;
}

void IECTransreceiver::goToIdleState() {
  setState(STATE_IDLE); // Non addressed device goes back to idle state
  writePin(DATA_PIN, false);
  writePin(CLOCK_PIN, false);
}

void IECTransreceiver::beginAttention() {
  Serial.print('a');

  // Stop in what we are doing and listen
  stopTimer();
  listener = false;
  talker = false;
  attention = true;
  resetDataBuffer();
  setState (STATE_WAIT_FOR_TALKER);

  writePin(CLOCK_PIN, false);
  enableInterrupt(CLOCK_INTERRUPT);
  disableInterrupt(DATA_INTERRUPT);
  writePin(DATA_PIN, true);
}

void IECTransreceiver::endAttention() {
  Serial.print('A');
  attention = false;

  if (state == STATE_DATA_RECEIVED && dataBufferLength > 0) { // Last frame of the attention message received
    //printBuffer();

    if (dataBuffer[0] >= 0x20 && dataBuffer[0] <= 0x3F && dataBuffer[0] - 0x20 == DEVICE_NUMBER) { // LISTEN
      processListenCommand();
    } else if (dataBuffer[0] >= 0x40 && dataBuffer[0] <= 0x5F && dataBuffer[0] - 0x40 == DEVICE_NUMBER) { // TALK
      processTalkCommand();
    } else if (dataBuffer[0] == 0x3F) { // UNLISTEN
      processUnlistenCommand();
    } else if (dataBuffer[0] == 0x5F) { // UNTALK
      processUntalkCommand();
    } else {
      goToIdleState(); // Non addressed device goes back to idle state
    }
  }
}

void IECTransreceiver::processTalkCommand() {
  channelToTalk = dataBuffer[1] - 0x60;
  resetDataBuffer();
  setState(STATE_BECOMING_TALKER);
}

void IECTransreceiver::processListenCommand() {
  Serial.println(); Serial.print('L');
  listener = true;
  talker = false;
  if (dataBuffer[1] >= 0xE0 && dataBuffer[1] <= 0xEE) { // OPEN the file/data channel with name transmitted next
    byte channelNeedsOpening = dataBuffer[1] - 0xE0;
    // TODO: signal file/data open to other class
  } else if (dataBuffer[1] >= 0xF0 && dataBuffer[1] <= 0xFE) { // CLOSE the file/data channel
    byte channelNeedsClosing = dataBuffer[1] - 0xF0;
    // TODO: signal file/data close
  }

  resetDataBuffer();
  setState (STATE_WAIT_FOR_TALKER);
  // Data line is being hold down here by us, which is exactly we need.
}

void IECTransreceiver::processIncomingBit() {
  int dataBit = readPin(DATA_PIN);
  if (dataBit != LOW) {
    currentDataByte |= (1 << currentDataBit);
  }
  currentDataBit++;

  if (currentDataBit == 8) { // We got a frame of a byte
    dataBuffer[dataBufferLength] = currentDataByte;

    writePin(DATA_PIN, true); // Signal that we have received the byte
    dataBufferLength = (dataBufferLength + 1) % DATA_BUFFER_SIZE;

    if (eoiReceived) { // Both talker and listener ends current transmission
      delayMicroseconds(60);
      writePin(DATA_PIN, false); // Release the data line
      setState (STATE_IDLE);
      if (!attention) {
        processData();
      }
      resetDataBuffer();
    } else {
      setState (STATE_DATA_RECEIVED);
    }
  }
}

void IECTransreceiver::doTalkerTurnaround() {
  disableInterrupt(CLOCK_INTERRUPT); // TALKER will generate clock so we are not interested about changes
  writePin(DATA_PIN, false);
  writePin(CLOCK_PIN, true);
  talker = true;
  listener = false;
  delayMicroseconds(80); // Talker acknowledge
  setState(STATE_TALKER_IDLE);
  Serial.println(); Serial.print('T');

  if (readyToSendCallback != NULL) {
    readyToSendCallback();
  } else {
    releaseClockLine(); // Triggers file not found
  }
}

void IECTransreceiver::processUnlistenCommand() {
  Serial.println('l');
  listener = false;
  goToIdleState();
}

void IECTransreceiver::processUntalkCommand() {
  Serial.println('t');
  talker = false;
  goToIdleState();
}

void IECTransreceiver::confirmEoi() {
  Serial.print('e');
  // EOI signaled
  stopTimer();
  eoiReceived = true;
  // EOI Timeout handshake
  writePin(DATA_PIN, true);
  delayMicroseconds(120);
  writePin(DATA_PIN, false);
}

void IECTransreceiver::startTransmission() {
  setDriveLed(true);
  disableInterrupt(DATA_INTERRUPT);
  setState(STATE_TALKER_WRITING_DATA_BITS);
  startTimer(40);
}

void IECTransreceiver::sendNextBit() {
  if (currentDataBit == 8) {
    writePin(DATA_PIN, false); // Release data line
    setState(STATE_TALKER_WAIT_FOR_FRAME_ACKNOWLEDGEMENT);
    enableInterrupt(DATA_INTERRUPT);
    writePin(CLOCK_PIN, true); // Listener should send acknowledgment now
  } else {
    writePin(CLOCK_PIN, true); // preparing to send the next bit
    writePin(DATA_PIN, !(currentDataByte & (1 << currentDataBit)));

    delayMicroseconds(70); // wait 70us to data become stable
    writePin(CLOCK_PIN, false); // listener now becomes noticed that the data is stable

    currentDataBit++;
    startTimer(60);
  }
}

void IECTransreceiver::finishTransmission() {
  // Listener acknowledged the frame
  if (sendingWithEoi) {
    delayMicroseconds(60);
  } else {
    delayMicroseconds(100); // wait time between bytes
  }
  setDriveLed(false);
  setState(STATE_TALKER_IDLE);
}

void IECTransreceiver::setState(int newState) {
  state = newState;
}

void IECTransreceiver::beginReceivingData() {
  currentDataByte = 0;
  currentDataBit = 0;
  setState(STATE_READY_FOR_DATA);
  eoiReceived = false;
  writePin(DATA_PIN, false); // Release the Data line and flag that we are listening.
  if (!attention) {
    startTimer(250);
  }
}

void IECTransreceiver::resetDataBuffer() {
  dataBufferLength = 0;
}

void IECTransreceiver::printBuffer() {
  for (int i = 0; i < dataBufferLength; i++) {
    Serial.print('/');
    Serial.print(dataBuffer[i], HEX);
  }
}

void IECTransreceiver::processData() {
  printBuffer();
}
