/* _____                 _           _              __  __       _
  |  __ \               | |         | |            |  \/  |     | |
  | |__) |__ ___   _____| |__   ___ | |_   ______  | \  / | ___ | |_ ___  _ __ ___
  |  _  // _` \ \ / / _ \ '_ \ / _ \| __| |______| | |\/| |/ _ \| __/ _ \| '__/ __|
  | | \ \ (_| |\ V /  __/ |_) | (_) | |_           | |  | | (_) | || (_) | |  \__ \
  |_|  \_\__,_| \_/ \___|_.__/ \___/ \__|          |_|  |_|\___/ \__\___/|_|  |___/            */

#include<Arduino.h>
#include<Keypad.h>
#include<Wire.h>
#include<Adafruit_LEDBackpack.h>
#include<Adafruit_PWMServoDriver.h>
#include<SoftwareSerial.h>
#include<SabertoothSimplified.h>
#include<Cytron_PS2Shield.h>

unsigned long timey;

// movement control vars
bool nodding = false;
bool shaking = false;
bool lClawOpening = true;
bool lClawMoving = false;
unsigned long noddingTime;
unsigned long shakingTime;


// switches in arade buttons
int switchPins[14] = { 27, 29, 31, 33, 35, 37, 39, 41, 45, 43, 53, 51, 49, 47 };

// Big main motor driver
SoftwareSerial SabretoothSerial(NOT_A_PIN, 9); // RX on no pin (unused), TX on pin 9 (to S1).
SoftwareSerial SabretoothSerial2(NOT_A_PIN, A9); // RX on no pin (unused), TX on pin 9 (to S1).
// NOTE TO JR - you just wired the other sabertooth to pin 52
SabertoothSimplified ST(SabretoothSerial); // Use SoftwareSerial as the serial port.
SabertoothSimplified ST2(SabretoothSerial2); // Use SoftwareSerial as the serial port.

Cytron_PS2Shield ps2(10, 11);

// 7 segmehnt led setup
Adafruit_LEDBackpack matrix = Adafruit_LEDBackpack();

// PCA9685 Led driver for arcade buttons
Adafruit_PWMServoDriver ledPwm = Adafruit_PWMServoDriver(0x40);

// PCA9685 Servo driver for arms, you can't do much if you got no arms.
Adafruit_PWMServoDriver servoPwm = Adafruit_PWMServoDriver(0x41);

// Keypad setup
char keys[4][3] = { {'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}};
byte rowPins[4] = {5, 6, 7, 8}; //connect to the row pinouts of the keypad
byte colPins[3] = {2, 3, 4}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, 4, 3);

void setup() {
  delay(500);

  // Send midi to debug
  //Serial.begin(9600);

  // Talk to the other arduino
  Serial2.begin(57600);

  // make random more random?!
  randomSeed(analogRead(0));

  ps2.begin(57600);

  // The relays which control the arms
  pinMode(22, OUTPUT);
  pinMode(24, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(28, OUTPUT);
  digitalWrite(22, HIGH);
  digitalWrite(24, HIGH);
  digitalWrite(26, HIGH);
  digitalWrite(28, HIGH);

  SabretoothSerial.begin(9600); // Set the same as the baud pins on the sabretooth.
  SabretoothSerial2.begin(9600); // Set the same as the baud pins on the sabretooth.

  // The jukebox 7segment
  matrix.begin(0x71);  // pass in the address
  matrix.setBrightness(255);

  // servo
  servoPwm.begin();
  servoPwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  initServos();

  ledPwm.begin();
  ledPwm.setPWMFreq(1600);  // This is the maximum PWM frequency

  // Arm cutoff
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);

  // Arcade switch
  pinMode(switchPins[0], INPUT_PULLUP);
  pinMode(switchPins[1], INPUT_PULLUP);
  pinMode(switchPins[2], INPUT_PULLUP);
  pinMode(switchPins[3], INPUT_PULLUP);
  pinMode(switchPins[4], INPUT_PULLUP);
  pinMode(switchPins[5], INPUT_PULLUP);
  pinMode(switchPins[6], INPUT_PULLUP);
  pinMode(switchPins[7], INPUT_PULLUP);
  pinMode(switchPins[8], INPUT_PULLUP);
  pinMode(switchPins[9], INPUT_PULLUP);
  pinMode(switchPins[10], INPUT_PULLUP);
  pinMode(switchPins[11], INPUT_PULLUP);
  pinMode(switchPins[12], INPUT_PULLUP);
  pinMode(switchPins[13], INPUT_PULLUP);
}

void loop()
{
  timey = millis();

  talkToLights();

  doServos();

  doJukebox();

  doMyArms();

  doMyWheels();

  doArcadeBtn();
}


