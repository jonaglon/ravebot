char strIn[4];
char strOut[4];

void talkToLights() {
  
  receiveSerialFromLights();

  checkLedIntensitySendChangeToLights();

  checkAnalogsForEyeMovements();

  checkButtonsSendInfoToLights();
}

void receiveSerialFromLights() {

  while (Serial3.available()) {
    int i=0;
    delay(5); //allows all serial sent to be received together
    while(Serial3.available() && i<4) {
      strIn[i++] = Serial3.read();
    }
    strIn[i++]='\0';
    int lightsMessage = atoi(strIn);
    doSomethingWithMessageFromLights(lightsMessage);
  }
}

void doSomethingWithMessageFromLights(int messageFromLights) {
  int requestFunction = messageFromLights / 1000;
  int requestMessage = messageFromLights % 1000;
  
    if (testMode) {
      Serial.print("Received Serial 2 Func:");
      Serial.print(requestFunction);
      Serial.print("   Received message:");
      Serial.println(requestMessage);
    }

  if (requestFunction == 1) // this is a beat message
  {
    for (int switchNum = 0; switchNum < 14; switchNum++) {
      if (switchNum==requestMessage) {
        ledPwm.setPWM(switchNum, 0, 0);
      }
    }
  }
  else if (requestFunction == 2) // this is a message to tell us what song is playing
  {
    currentSegmentNum = requestMessage;
    showNumber();
    if (testMode) {
      Serial.print("   * ** Play song:");
      Serial.println(requestMessage);
    }
  }
}

void checkLedIntensitySendChangeToLights() {
  if (timey > nextAnalogRead) {
    int sensorValue = analogRead(A8);
    int newIntensity = 55-(sensorValue/19); // should give us a range ~2-50
    if ((newIntensity > ledIntensity+1 || newIntensity < ledIntensity-1) && newIntensity > 2) {
      ledIntensity = newIntensity;
      sendSerialToLights(3, ledIntensity);
    }
    nextAnalogRead = timey+300;
  }  
}

int leftEyeX;
int leftEyeY;
int rightEyeX;
int rightEyeY;
void checkAnalogsForEyeMovements() {

  if (ps2.readButton(PS2_LEFT_2) == 0) {  
    int newLeftX=ps2.readButton(PS2_JOYSTICK_LEFT_X_AXIS)/4;
    if (newLeftX > leftEyeX || newLeftX < leftEyeX) {
      leftEyeX = newLeftX;
      sendSerialToLights(7, leftEyeX);
    }
    
    int newLeftY=ps2.readButton(PS2_JOYSTICK_LEFT_Y_AXIS)/4;
    if (newLeftY > leftEyeY || newLeftY < leftEyeY) {
      leftEyeY = newLeftY;
      sendSerialToLights(8, leftEyeY);
    }    

    int newRightX=ps2.readButton(PS2_JOYSTICK_RIGHT_X_AXIS)/4;
    if (newRightX > rightEyeX || newRightX < rightEyeX) {
      rightEyeX = newRightX;
      sendSerialToLights(5, rightEyeX);
    }
    
    int newRightY=ps2.readButton(PS2_JOYSTICK_RIGHT_Y_AXIS)/4;
    if (newRightY > rightEyeY || newRightY < rightEyeY) {
      rightEyeY = newRightY;
      sendSerialToLights(6, rightEyeY);
    }    
    
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
  
  itoa(value, strOut, 10); //Turn value into a character array
  Serial1.write(strOut, 4);

}

