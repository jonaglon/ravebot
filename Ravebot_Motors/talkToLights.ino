char str[4];

void talkToLights() {
  
  receiveSerialFromLights();

  //checkLedIntensitySendChangeToLights();

  checkButtonsSendInfoToLights();
}

void receiveSerialFromLights() {

  while (Serial2.available()) {
    int i=0;
    delay(5); //allows all serial sent to be received together
    while(Serial2.available() && i<4) {
      str[i++] = Serial2.read();
    }
    str[i++]='\0';
    doSomethingWithMessageFromLights(atoi(str));
  }
}

void doSomethingWithMessageFromLights(int messageInt) {
  int function = messageInt / 1000;
  int message = messageInt % 1000;
  
    if (testMode) {
      Serial.print("Received Serial 2 Func:");
      Serial.print(function);
      Serial.print("   Received message:");
      Serial.print(message);
    }

  if (function == 1) // this is a beat message
  {
    for (int switchNum = 0; switchNum < 14; switchNum++) {
      if (switchNum==message) {
        ledPwm.setPWM(switchNum, 0, 0);
      }
    }
  }
  else if (function == 2) // this is a message to tell us what song is playing
  {
    // todo - really have to uncomment this
    // setDisplay(message);
  }
}

void checkLedIntensitySendChangeToLights() {

  if (nextAnalogRead > timey) {
    int sensorValue = analogRead(A8);
    int newIntensity = 50-(sensorValue/19); // should give us a range ~2-50
    if (newIntensity != ledIntensity) {
      ledIntensity = newIntensity;
      sendSerialToLights(3, ledIntensity);
    }    
    nextAnalogRead = timey+300;
  }  
}

// bool ps2Right1On = false;
bool ps2Right2On = false;
void checkButtonsSendInfoToLights() {
  
  if (!ps2Right2On && ps2.readButton(PS2_RIGHT_2)==0)
  {
    ps2Right2On = true;
    sendSerialToLights(1, 0);
  }
  if (ps2Right2On && ps2.readButton(PS2_RIGHT_2)==1)
  {
    ps2Right2On = false;
    sendSerialToLights(1, 1);
  }
  
  /* if (!ps2Right2On && ps2.readButton(PS2_RIGHT_2)==0)
  {
    ps2Right2On = true;
    sendSerialToLights(3, 0);
  }
  if (ps2Right2On && ps2.readButton(PS2_RIGHT_2)==1)
  {
    ps2Right2On = false;
    sendSerialToLights(3, 1);
  } */
  
}


void sendSerialToLights(int function, int message) {

  if (testMode) {
    Serial.println("Sending to Serial 2");
  }


  int value = (function * 1000) + message;
  //Serial.println(message);
  
  itoa(value, str, 10); //Turn value into a character array
  Serial1.write(str, 4);

}

