int readPs2Var;

void doMyArms() {
  int rightUp = digitalRead(A5);
  int rightDown = digitalRead(A4);
  int leftUp = digitalRead(A2);
  int leftDown = digitalRead(A3);

  // Right arm
  if (rightUp==0 && rightDown==0) {
    sendRArmMotorValue(0);
  } else if (rightUp==0) {
    sendRArmMotorValue(-80);
    delay(100);
    sendRArmMotorValue(0);
  } else if (rightDown==0) {
    sendRArmMotorValue(80);
    delay(100);
    sendRArmMotorValue(0);
  } else if (ps2.readButton(PS2_LEFT_1) == 0) {
    readPs2Var=-(ps2.readButton(PS2_JOYSTICK_LEFT_Y_AXIS)-128);
    if (readPs2Var < 2) {
      sendRArmMotorValue(readPs2Var);
    } else if (readPs2Var > 2) {
      sendRArmMotorValue(readPs2Var);
    } else {
      sendRArmMotorValue(0);
    }
  }

  // Left arm
  if (leftUp==0 && leftDown==0) {
    sendLArmMotorValue(0);
  } else if (leftUp==0) {
    sendLArmMotorValue(80);
    delay(100);
    sendLArmMotorValue(0);
  } else if (leftDown==0) {
    sendLArmMotorValue(-80);
    delay(100);
    sendLArmMotorValue(0);
  } else if (ps2.readButton(PS2_LEFT_1) == 0) {
    readPs2Var=ps2.readButton(PS2_JOYSTICK_RIGHT_Y_AXIS)-128;
    if (leftUp==1 && readPs2Var > 2) {
      sendLArmMotorValue(readPs2Var);
    } else if (leftDown==1 && readPs2Var < 2) {
      sendLArmMotorValue(readPs2Var);
    } else {
      sendLArmMotorValue(0);
    }
  }
  
}    

int rArmMotorValue = 0;
void sendRArmMotorValue(int newValue) {
  if (newValue != rArmMotorValue)
  {
    rArmMotorValue = newValue;
    ST1.motor(2, rArmMotorValue);
  }
}

int lArmMotorValue = 0;
void sendLArmMotorValue(int newValue) {
  if (newValue != lArmMotorValue)
  {
    lArmMotorValue = newValue;
    ST1.motor(1, lArmMotorValue);
  }
}

void doMyWheels() {

  if (ps2.readButton(PS2_RIGHT_1) == 0) {
    readPs2Var=(ps2.readButton(PS2_JOYSTICK_LEFT_Y_AXIS)-128)/4;

    //Serial.print("ly:");  
    //Serial.print(readPs2Var);  
    ST2.motor(1, readPs2Var);

    readPs2Var=(ps2.readButton(PS2_JOYSTICK_RIGHT_Y_AXIS)-128)/4;
    //Serial.print("   ry:");  
    //Serial.println(readPs2Var);  
    ST2.motor(2, readPs2Var);
  }
}
