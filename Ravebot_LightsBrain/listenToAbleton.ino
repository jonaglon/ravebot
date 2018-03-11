byte incomingByte;
byte note;
byte velocity;
int channel = 1;
int noteDown = LOW;
int state=0;  // state machine variable 0 = command waiting : 1 = note waitin : 2 = velocity waiting

// Receive midi from ableton
void listenToAbleton() {

  if (beatTestMode)
  {
    doFakeBeatMessageFromAbleton();
    return;
  }

  while (Serial.available() > 0) {
    incomingByte = Serial.read();
    switch (state) {
    case 0:
      // look for as status-byte, our channel, note on
      if (incomingByte== (144 | channel)) { 
        state=1;
      }
      // look for as status-byte, our channel, note off
      if (incomingByte== (128 | channel)) { 
        state=1;
      }

    case 1:
      // get the note to play or stop
      if(incomingByte < 128) {
        note=incomingByte;
        state=2; 
      }
      else {
        state = 0;  // reset state machine as this should be a note number
      }
      break;

    case 2:
      // get the velocity
      if(incomingByte < 128) {
        processMessageFromAbleton(note, incomingByte, noteDown);
      }
      state = 0;  // reset state machine to start            
    }
  }
}

unsigned long lastHalfBeatTime = 0;
int lastHalfBeatLength = 1;
void setBeatTimes() {
  lastHalfBeatLength = timey-lastHalfBeatTime; 
  lastHalfBeatTime = timey;
}

void processMessageFromAbleton(byte note, byte velocity, int down) {
  if ((note>=24 && note<88) && (velocity == 100)) {
    sixteenHalfBeats = note - 24; // from 0 to 63 

    setBeatTimes();
    
    if (testMode) {
      Serial.print("16: ");
      Serial.println(sixteenHalfBeats);
    }
    sendBeatToMega();
    if (dropCountdown != 0)
      dropCountdown--;

    if (sixteenHalfBeats % 8 == 0) {
      if (testMode) {
        Serial.print("New bar: ");
        Serial.println(currentBar);
        Serial.print("CurrentTuneBpm:");
        Serial.print(currentTune.bpm);
        Serial.print("  NextTuneBpm:");
        Serial.println(nextTune.bpm);
      }
      // This is the beginning of a new bar, might need to end a mix or start a drop countdown
      currentBar++;
      mixCurrentBar++;
      checkForDropCountdownStart();
      checkForMixEnd();
    }
    
    if (currentlyMixing) {
      setPercentThroughMix();
      doMixing();
    }

    if (sixteenHalfBeats % 8 == 6) {
      checkForMixStart();
    }
    
  }
}

/*void checkForQuantisationStart() {
  if (currentBar == nextMixStart) {
    if (testMode)
        Serial.println("sending quantisation on");
    sendQuantisationOn();
  }
}

void checkForQuantisationEnd() {
  if (currentBar == nextMixStart+1) {
    sendQuantisationOff();
  }
}*/

void checkForMixStart() {
  if (testMode) {
    Serial.print("checking for mix start. current bar:");
    Serial.print(currentBar);
    Serial.print("  next mix start:");
    Serial.println(nextMixStart);
  }
    
  if ((currentBar+1 == nextMixStart) && nextMixDuration == 0) {
    doImmediateMix();
  } else if (currentBar+1 == nextMixStart) {
    startNewMix();
  }
}

void checkForMixEnd() {
  if (currentBar == nextMixStart + nextMixDuration)
  {
    endMixAndPickNewTune();
  }
}

void checkForDropCountdownStart() {
  if (currentBar+4 ==  currentTune.drop)
    dropCountdown = 16;
  
  if (currentBar+2 ==  currentTune.drop)
    dropCountdown = 8;
}


/* Below is the fake beat control code */
unsigned long nextBeat = 0;

void doFakeBeatMessageFromAbleton() {
  if (timey > nextBeat) {
    processMessageFromAbleton(((fakeBeatCount+2)%16)+24, 100, 0);
    nextBeat = timey+fakeBeatLengh;
    fakeBeatCount++;
  }
}


