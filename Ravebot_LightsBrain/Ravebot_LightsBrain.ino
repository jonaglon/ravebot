  /* __                 _           _               __ _       _     _  
  /__\ __ ___   _____| |__   ___ | |_            / /(_) __ _| |__ | |_ ___
 / \/// _` \ \ / / _ \ '_ \ / _ \| __|  _____   / / | |/ _` | '_ \| __/ __|
/ _  \ (_| |\ V /  __/ |_) | (_) | |_  |_____| / /__| | (_| | | | | |_\__ \
\/ \_/\__,_| \_/ \___|_.__/ \___/ \__|         \____/_|\__, |_| |_|\__|___/
                                                       |___/                       */
#include<Arduino.h>
#include<Wire.h>
#include<FastLED.h>

bool testMode = false;

unsigned long timey;
unsigned long fakeBeatCount = 0; 
int ticky=0;

unsigned long beatTimes[10] = {0,0,0,0,0,0,0,0,0,0};
int fakeBeatLengh = 240;

// Set by midi in to be 1-16 with beat.
short sixteenBeats = 0;

int mainVolume = 100; // 127 actual max but we won't exceed 100.
int currentBar = 0;
int newCurrentBar = 0; // This counts from the start of a mix
int currentGenre = 0;
int currentTrack = 0;
int dropCountdown = 0;

bool robotTalking = false;
unsigned long robotTalkingOnTime;

const int numLeds = 671;
const int numLedsAdj = (numLeds * 4) / 3;
CRGB rgbwLeds[numLedsAdj];

// MIXING VARS
int nextTrack = 0;
int nextGenre = 0;
int nextMixDuration = 0;
int abletonBpm = 0;
bool stayWithinGenre = false;
bool currentlyMixing=false;
bool deckASelected = true;
int currentMixerPosition = 0;

void setup() {
  delay(500);

  // Talk to Ableton using midi over USB, or debug.
  Serial.begin(9600);

  // Listen to the other arduino
  Serial2.begin(57600);
  // Talk to the other arduino
  Serial3.begin(9600);

  // Make random more random?!
  randomSeed(analogRead(0));

  pinMode(12, OUTPUT); // rgb LED Setup
  LEDS.addLeds<WS2812B, 12>(rgbwLeds, numLedsAdj);
  LEDS.setBrightness(255); // 255 max

  setMainVolume(mainVolume);

}

void loop()
{
  timey = millis();

  listenToAbleton();

  receiveFromMega();

  doLights();

}





struct tuneInfo {
  byte bpm;
  byte drop1;
  byte drop2;
  byte tuneLength;
  byte minFadeIn;
  byte maxFadeIn;
  byte minFadeOut; 
  byte maxFadeOut;
  tuneInfo(byte aBpm, byte aDrop1, byte aDrop2, byte aTuneLength, byte aMinFadeIn, byte aMaxFadeIn, byte aMinFadeOut, byte aMaxFadeOut) : 
           bpm(aBpm), drop1(aDrop1), drop2(aDrop2), tuneLength(aTuneLength), minFadeIn(aMinFadeIn), maxFadeIn(aMaxFadeIn), minFadeOut(aMinFadeOut), maxFadeOut(aMaxFadeOut) { 
  }
};

int numTunesByGenre[8] = {6, 5, 5, 5, 5, 5, 5, 5};

tuneInfo tunesLibrary[4][6] = {
 {{101,  5, 25,  24, 0, 4, 4, 8},  // Lets get ill                       // 1   Hip-hop
  { 93,  9, 49,  20, 0, 4, 8, 8},  // No Diggidy 
  {100,  5, 25,  20, 0, 8, 4, 8},  // Like it raw
  {103,  5, 69,  16, 0, 4, 0, 8},  // Moma said knock you out          
  {162,  7, 65,  16, 0, 0, 4, 8},  // Dead Prez
  { 93,  3, 61,  24, 4, 8, 4, 8}}, // Close to me snoop  

 {{102,  9, 97, 137, 4, 4, 4, 4},  // Aphex Ageopolis  
  {103,  7, 83, 111, 4, 4, 4, 4},  // Whitetown I could never
  { 96,  9, 53,  76, 4, 4, 4, 4},  // DM Big L 
  {117,  9, 29,  75, 4, 4, 4, 4},  // Air Remember
  {172,  0,  0, 152, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4}},  // DM Zero7

 {{ 86,  9,  0,  51, 4, 4, 4, 4},  // Tenor Saw Ring the Alarm
  {102, 49, 61, 119, 4, 4, 4, 4},  // Toots Funky Kingston
  { 80,  0,  0,  79, 4, 4, 4, 4},  // WayneSmith - UnderMeSleng Teng
  { 86,  0,  0,  67, 4, 4, 4, 4},  // Sis Nancy Bam Bam 
  { 83,  0,  0,  77, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4}},   // Althea&Donna Strictly Roots
 
 {{160,  0,  0, 113, 4, 4, 4, 4}, // Kim Wilde - Kids in America
  {126,  0,  0,  81, 4, 4, 4, 4}, //Kylie - cant get you out
  {112,  0,  0, 101, 4, 4, 4, 4}, //Hall&Oates - I can't go for that
  { 97,  0,  0,  63, 4, 4, 4, 4}, //George Michael - Faith
  {122, 49, 87, 107, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4}} //DeeLite - Groove is in the heart
};

/*
tuneInfo tunesLibrary[8][96] = {
 {{101,  5, 25,  12, 4, 4, 4, 4},  // Lets get ill
  { 93,  9, 49,  12, 4, 4, 4, 4},  // No Diggidy
  {103,  5, 69,  12, 4, 4, 4, 4},  // Moma said knock you out        // 0, 1   Hip-hop
  {100,  5, 25,  12, 4, 4, 4, 4},  // Like it raw
  { 92,  2, 49,  12, 4, 4, 4, 4},  // Dre&2Pac California
  {162,  7, 65, 143, 4, 4, 4, 4},  // Bigger than hip hop
  { 93,  3, 61,  73, 4, 4, 4, 4},  // Close to me snoop
  {160, 77,  0, 159, 4, 4, 4, 4},  // Coolio Gansters paradise
  {100,  5, 47,  89, 4, 4, 4, 4},  // HipHopHooray
  {108,  4, 34,  96, 4, 4, 4, 4},  // Jump Around 
  {103,  0,  0,  81, 4, 4, 4, 4},  // Insazlle in the bazzle - Cyprus Hazzle
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4}},  // Undefined

 {{102,  9, 97, 137, 4, 4, 4, 4},  // Aphex Ageopolis  
  {103,  7, 83, 111, 4, 4, 4, 4},  // Whitetown I could never
  { 96,  9, 53,  76, 4, 4, 4, 4},  // DM Big L 
  {117,  9, 29,  75, 4, 4, 4, 4},  // Air Remember
  {172,  0,  0, 152, 4, 4, 4, 4},  // DM Zero7
  { 92,  9, 33,  52, 4, 4, 4, 4},  // BOC Roygbiv 
  {127,  5, 33, 125, 4, 4, 4, 4},  // Royksop Remind me
  {105, 10, 38, 116, 4, 4, 4, 4},  // Chakka Aint nobody
  {140, 41, 93, 129, 4, 4, 4, 4},  // FOTC The humans are dead
  { 87,  5, 49,  89, 4, 4, 4, 4},  // Roos Manuva Again and again
  {115,  0,  0, 142, 4, 4, 4, 4}, // Jamacian Boy - Lone Ranger 
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4}},  // Undefined

 {{ 86,  9,  0,  51, 4, 4, 4, 4},  // Tenor Saw Ring the Alarm
  {102, 49, 61, 119, 4, 4, 4, 4},  // Toots Funky Kingston
  { 80,  0,  0,  79, 4, 4, 4, 4},  // WayneSmith - UnderMeSleng Teng
  { 86,  0,  0,  67, 4, 4, 4, 4},  // Sis Nancy Bam Bam
  { 83,  0,  0,  77, 4, 4, 4, 4},   // Althea&Donna Strictly Roots
  { 77, 27,  0,  52, 4, 4, 4, 4},   // Damian Marley Welcome To Jamrock
  {147, 13, 59, 147, 4, 4, 4, 4},  // Tanya S - It's a pity
  { 88,  0,  0,  72, 4, 4, 4, 4},  // Marcia G - Feel like jumping
  { 82,  0,  0,  72, 4, 4, 4, 4},  // Cant stop now, MajorLazer
  {148,  0,  0, 106, 4, 4, 4, 4},  // Toots - pressure drop
  {104,  0,  0,  99, 4, 4, 4, 4}, // Bob - Could you be loved
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4}},  // Undefined
   
 {{160,  0,  0, 113, 4, 4, 4, 4}, // Kim Wilde - Kids in America
  {126,  0,  0,  81, 4, 4, 4, 4}, //Kylie - cant get you out
  {112,  0,  0, 101, 4, 4, 4, 4}, //Hall&Oates - I can't go for that
  { 97,  0,  0,  63, 4, 4, 4, 4}, //George Michael - Faith
  {122, 49, 87, 107, 4, 4, 4, 4}, //DeeLite - Groove is in the heart
  {126,  0,  0, 103, 4, 4, 4, 4}, //Euritmics - sweet dreams
  {126, 39,  0,  97, 4, 4, 4, 4}, //PaulSimon&Dylan Stuck in the middle
  {164,  9, 65, 136, 4, 4, 4, 4}, //Martha&Muffins Echo Beach
  {199,  0,  0, 111, 4, 4, 4, 4}, //TheCoral - DreamingOfYou
  { 99,  0,  0,  58, 4, 4, 4, 4}, // Blister in the sun - Voilent femmes
  {114,  0,  0,  96, 4, 4, 4, 4}, // Erasure a little respect
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4}},  // Undefined
   
 {{133, 29, 53, 120, 4, 4, 4, 4}, // A rinky dinky
  {136,  5, 45,  93, 4, 4, 4, 4}, // Israelites mix
  {108, 57,  0,  83, 4, 4, 4, 4}, // Kelis - Trick Me
  {150,  9, 25, 125, 4, 4, 4, 4}, // Dubbleedge - Lips to the floor
  {134, 17,105, 121, 4, 4, 4, 4}, // Zero Emit Collect
  {125, 29, 71, 103, 4, 4, 4, 4}, // Dizee Bonkers
  {115, 21, 57,  84, 4, 4, 4, 4}, // RizzleKicks - DownWithTheTrumpets
  {168, 29, 93, 173, 4, 4, 4, 4}, // OneTime
  {130,  0,  0, 161, 4, 4, 4, 4}, // WileOut - DJ Zinc
  {145,  0,  0, 171, 4, 4, 4, 4}, // Aphex - Polynomial C
  {123,  0,  0, 187, 4, 4, 4, 4},// Aphex WindowLicker
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4}},  // Undefined
  
 {{ 85,  0,  0,  84, 4, 4, 4, 4}, // Costa del essex
  {132,  0,  0, 139, 4, 4, 4, 4}, // MC Hammer, cant touch
  {118,  8,  0,  34, 4, 4, 4, 4}, // Minder
  {172,  0,  0, 112, 4, 4, 4, 4}, // TheClappingSong
  {120,  0,  0,  87, 4, 4, 4, 4}, // Killer Queen
  {170,  0,  0,  89, 4, 4, 4, 4}, // Creep RichardCheese
  {170,  0,  0, 132, 4, 4, 4, 4}, // Crash Primitives
  { 94,  0,  0,  57, 4, 4, 4, 4}, // TooManyMuthaUkkas - FOTC
  {172,  0,  0,  89, 4, 4, 4, 4}, // HitTheRoadJack - RayCharles
  {112,  0,  0,  86, 4, 4, 4, 4}, // Crazy - GnarlesBarkley  
  {160,  0,  0,  93, 4, 4, 4, 4}, // DevilInDisguise - Elvis ********
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4}},  // Undefined

 {{120,  0,  0, 143, 4, 4, 4, 4}, // BackToMyRoots - RichieHavens
  {104,  0,  0, 116, 4, 4, 4, 4}, // Think - Aretha
  {100,  0,  0, 182, 4, 4, 4, 4}, // As - Wonder
  {174,  0,  0, 155, 4, 4, 4, 4}, // Roady - FatFreddyNextmen
  {130,  0,  0,  89, 4, 4, 4, 4}, // Beggin - FrankieValli *******
  { 98,  0,  0,  70, 4, 4, 4, 4}, // IGotAWoman - RayCharles
  {156,  0,  0, 143, 4, 4, 4, 4}, // MilkAndHoney-PrinceFatty
  {124,  0,  0, 123, 4, 4, 4, 4}, // BackToBlack - Amy 
  {132,  0,  0, 166, 4, 4, 4, 4}, // MasterBlaster-StevieWonder
  {109,  0,  0, 113, 4, 4, 4, 4}, // AllNightLong - LionelRichie
  { 96,  0,  0,  97, 4, 4, 4, 4}, // INeedADollar-AloeBlacc     {122,  0,  0, 124} // GotToGiveItUp-MarvinGaye
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4}},  // Undefined

 {{124,  0,  0,  71, 4, 4, 4, 4}, // ILoveTheNightlife - Alecia Bridges
  {115,  0,  0, 105, 4, 4, 4, 4}, // LoveHangover - DianaRoss
  {110,  0,  0, 151, 4, 4, 4, 4}, // LastNightADjSavedMyLife-Indeep
  {134,  0,  0, 148, 4, 4, 4, 4}, // LayAllYourLoveOnMe-Abba
  {121,  0,  0, 182, 4, 4, 4, 4}, // HotStuff-DonnaSummer
  {128,  0,  0, 109, 4, 4, 4, 4}, // RingMyBell-AnitaWard
  {128,  0,  0, 129, 4, 4, 4, 4}, // EverybodyDance-Chic
  {111,  0,  0,  97, 4, 4, 4, 4}, // GoodTimes-Chic
  {101,  0,  0, 110, 4, 4, 4, 4}, // ThinkingOfYou-SisSledge
  {115,  0,  0, 105, 4, 4, 4, 4}, // SheCantLoveYou-Chemise
  {112,  0,  0, 131, 4, 4, 4, 4}, // Automatic-PointerSisters   
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4},  // Undefined
  {100,  0,  0,  60, 4, 4, 4, 4}} };  // Undefined
*/
/*
  {124,  0,  0, 124, 4, 4, 4, 4}, // WhatCanYouDoForMe-UtahSaints
  {149,  0,  0, 177, 4, 4, 4, 4}, // TripToTheMoonPt2-Acen
  {119,  0,  0, 110, 4, 4, 4, 4}, // YouGotTheLove-TheSourceFtCandiStanton
  {126,  0,  0,  75, 4, 4, 4, 4}, // MrKirsNightmare-4Hero
  {137,  0,  0, 113, 4, 4, 4, 4}, // Bombscare-2BadMice
  {126,  0,  0, 138, 4, 4, 4, 4}, // LFO-LFO
  {132,  0,  0, 135, 4, 4, 4, 4}, // Infiltrate202-Altern8 
  {122,  0,  0, 119, 4, 4, 4, 4}, // DirtyCash-SoldOutMix
  {122,  0,  0, 133, 4, 4, 4, 4}, // Break4Love-Raze
  {126,  0,  0,  93, 4, 4, 4, 4}, // IsThereAnybodyOutThere-Bassheads
  {126,  0,  0,  86, 4, 4, 4, 4}, //PacificState-808State

  {140,  0,  0, 148, 4, 4, 4, 4},  // NextHype-TempaT
  {175,  0,  0, 191, 4, 4, 4, 4},  // DuppyMan-ChaseAndStatusCapleton
  {174,  0,  0, 264, 4, 4, 4, 4},  // TheNine-BadCompany
  {174,  0,  0, 166, 4, 4, 4, 4},  // GoldDigger-HighContrast
  {175,  0,  0, 189, 4, 4, 4, 4},  // ShakeUrBody-ShyFX
  {180,  0,  0, 185, 4, 4, 4, 4},  // LastNight-BennyPage
  {175,  0,  0, 170, 4, 4, 4, 4}, // PassMeTheRizla-Deekline
*/


/* ************************************************************** *
 *                                                               
 *                            LED PATTERNS
 *  
 * ************************************************************** */

int tapePattern[224][2] = {
  {4,6}, {4,5}, {4,4}, {4,0}, {4,1}, {4,2}, {4,7}, {4,10}, {4,9}, {4,8}, 
  {4, 3}, {3,83}, {3,82}, {3,81}, {3,80}, {3,79}, {3,78}, {3,77}, {3,76}, {3,75},   
  {3,74}, {3,73}, {3,72}, {3,71}, {3,70}, {3,69}, {3,68}, {3,67}, {3,66}, {3,65},   
  {3,64}, {3,63}, {3,62}, {3,61}, {3,60}, {3,59}, {3,58}, {3,57}, {3,56}, {3,55},   
  {3,54}, {3,53}, {3,52}, {3,51}, {3,50}, {3,49}, {3,48}, {3,47}, {3,46}, {3,45},   
  {3,44}, {3,43}, {3,42}, {3,41}, {3,40}, {3,39}, {3,38}, {3,37}, {3,36}, {3,35},   
  {3,34}, {3,33}, {3,32}, {3,31}, {3,30}, {3,29}, {3,28}, {3,27}, {3,26}, {3,25},   
  {3,24}, {3,23}, {3,22}, {3,21}, {3,20}, {3,19}, {3,18}, {3,17}, {3,16}, {3,15},   
  {3,14}, {3,13}, {3,12}, {3,11}, {3,10}, {3, 9}, {3, 8}, {3, 7}, {3, 6}, {3, 5},   
  {3, 4}, {3, 3}, {3, 2}, {3, 1}, {3, 0}, {5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4},   
  {5, 5}, {5, 6}, {5, 7}, {5, 8}, {5, 9}, {5,10}, {5,11}, {5,12}, {5,13}, {5,14}, 
  {5,15}, {5,16}, {5,17}, {5,18}, {5,19}, {5,20}, {5,21}, {5,22}, {5,23}, {5,24}, 
  {5,25}, {5,26}, {5,27}, {5,28}, {5,29}, {5,30}, {5,31}, {0,16}, {0,15}, {0,14}, 
  {0,13}, {0,12}, {0,11}, {0,10}, {0, 9}, {0, 8}, {0, 7}, {0, 6}, {0, 5}, {0, 4}, 
  {0, 3}, {0, 2}, {0, 1}, {0, 0}, {0,31}, {0,30}, {0,29}, {0,28}, {0,27}, {0,26}, 
  {0,25}, {0,24}, {0,23}, {0,22}, {0,21}, {0,20}, {0,19}, {0,18}, {0,17}, {0,44}, 
  {0,43}, {0,42}, {0,41}, {0,40}, {0,39}, {0,38}, {0,37}, {0,36}, {0,35}, {0,34}, 
  {0,33}, {0,32}, {0,31}, {0,55}, {0,54}, {0,53}, {0,52}, {0,51}, {0,50}, {0,49},
  {0,48}, {0,47}, {0,46}, {0,45}, {0,64}, {0,63}, {0,62}, {0,61}, {0,60}, {0,59},
  {0,58}, {0,57}, {0,56}, {0,71}, {0,70}, {0,69}, {0,68}, {0,67}, {0,66}, {0,65},
  {0,78}, {0,77}, {0,76}, {0,75}, {0,74}, {0,73}, {0,72}, {0,83}, {0,82}, {0,81},
  {0,80}, {0,79}, {0,78}, {1, 7}, {1,10}, {1, 9}, {1, 8}, {1, 3}, {1, 0}, {1, 1}, {1, 2}, {1, 6}, {1, 5}, {1, 4} };

int sweepPatterns[270][5] = {
{  0, 27,  2,  8,  6}, {  1, 30,  2, 10,  3}, {  2, 33,  3, 13,  2}, {  3, 36,  4, 17,  1}, {  4, 38,  6, 21,  1}, {  5, 39,  8, 24,  1}, {  6, 40, 10, 28,  2}, {  7, 41, 12, 31,  3},{  8, 41, 15, 34,  6},{  9, 41, 18, 36,  9},
{ 10, 40, 20, 38, 12}, { 11, 39, 22, 39, 15}, { 12, 38, 24, 39, 19}, { 13, 36, 27, 39, 23}, { 14, 33, 28, 38, 27}, { 15, 30, 29, 36, 30}, { 16, 27, 29, 34, 32}, { 17, 25, 29, 31, 35},{ 18, 23, 28, 28, 37},{ 19, 21, 27, 24, 38},
{ 20, 19, 24, 21, 38}, { 21, 17, 22, 17, 38}, { 22, 16, 20, 13, 37}, { 23, 15, 18, 10, 35}, { 24, 15, 15,  8, 32}, { 25, 15, 12,  5, 30}, { 26, 16, 10,  3, 27}, { 27, 17,  8,  2, 23},{ 28, 19,  6,  2, 19},{ 29, 21,  4,  2, 15},
{ 30, 23,  3,  3, 12}, { 31, 25,  2,  5,  9}, { 32, 27,  5, 10,  8}, { 33, 30,  5, 13, 00}, { 34, 33,  6, 17,  5}, { 35, 36,  8, 21,  5}, { 36, 37, 10, 24,  5}, { 37, 38, 12, 28, 00},{ 38, 38, 15, 31,  8},{ 39, 38, 18, 34, 11},
{ 40, 37, 20, 35, 15}, { 41, 36, 23, 35, 19}, { 42, 33, 24, 35, 23}, { 43, 30, 26, 34, 27}, { 44, 27, 26, 31, 30}, { 45, 25, 26, 28, 32}, { 46, 23, 24, 24, 34}, { 47, 21, 23, 21, 34},{ 48, 19, 20, 17, 34},{ 49, 18, 18, 13, 32},
{ 50, 18, 15, 10, 30}, { 51, 18, 12,  8, 27}, { 52, 19, 10,  6, 23}, { 53, 21,  8,  6, 19}, { 54, 23, 06,  6, 15}, { 55, 25,  5,  8, 11}, { 56, 27,  7, 13, 11}, { 57, 30,  8, 16,  8},{ 58, 33,  9, 21,  8},{ 59, 36, 12, 25,  8},
{ 60, 36, 15, 28, 11}, { 61, 36, 18, 31, 15}, { 62, 33, 21, 32, 19}, { 63, 30, 23, 31, 24}, { 64, 27, 23, 28, 28}, { 65, 25, 23, 25, 30}, { 66, 22, 21, 21, 30}, { 67, 21, 18, 16, 30},{ 68, 20, 15, 13, 28},{ 69, 21, 12, 10, 24},
{ 70, 22,  9,  9, 19}, { 71, 25,  8, 10, 15}, { 72, 27,  9, 15, 14}, { 73, 30, 10, 19, 11}, { 74, 33, 12, 23, 11}, { 75, 33, 15, 26, 14}, { 76, 33, 18, 28, 17}, { 77, 30, 20, 28, 22},{ 78, 27, 21, 26, 25},{ 79, 25, 20, 23, 27},
{ 80, 23, 18, 19, 27}, { 81, 22, 15, 15, 25}, { 82, 23, 12, 13, 22}, { 83, 25, 10, 13, 17}, { 84, 25, 13, 17, 19}, { 85, 25, 15, 19, 22}, { 86, 25, 17, 21, 23}, { 87, 27, 11, 17, 15},{ 88, 27, 13, 19, 17},{ 89, 27, 15, 21, 19},
{ 90, 27, 17, 23, 22}, { 91, 27, 19, 25, 23}, { 92, 30, 13, 21, 15}, { 93, 30, 15, 23, 17}, { 94, 30, 17, 25, 19}, { 95,  0, 64, 81, 52}, { 96,  0, 62, 80, 51}, { 97,  0, 60, 78, 49},{ 98,  0, 58, 76, 47},{ 99,  0, 57, 74, 45},
{100,  0, 55, 72, 43}, {101,  0, 53, 71, 42}, {102,  0, 51, 69, 40}, {103,  0, 50, 67, 38}, {104,  0, 48, 63, 36}, {105,  0, 46, 64, 34}, {106,  0, 44, 62, 33}, {107,  0, 43, 60, 31},{108,  0, 41, 58, 29},{109,  0, 39, 57, 28},
{110,  0, 38, 55, 27}, {111,  0, 36, 53, 25}, {112,  0, 34, 52, 23}, {113, 27, 71, 73, 72}, {114, 30, 71, 75, 69}, {115, 33, 72, 78, 68}, {116, 36, 73, 82, 67}, {117, 38, 75, 86, 67},{118, 39, 77, 89, 67},{119, 40, 79, 93, 68},
{120, 41, 81, 96, 69}, {121, 41, 84, 99, 72}, {122, 41, 87,101, 75}, {123, 40, 89,103, 78}, {124, 39, 91,104, 81}, {125, 38, 93,104, 85}, {126, 36, 96,104, 89}, {127, 33, 97,103, 93},{128, 30, 98,101, 96},{129, 27, 98, 99, 98},
{130, 25, 98, 96,101}, {131, 23, 97, 93,103}, {132, 21, 96, 89,104}, {133, 19, 93, 86,104}, {134, 17, 91, 82,104}, {135, 16, 89, 78,103}, {136, 15, 87, 75,101}, {137, 15, 84, 73, 98},{138, 15, 81, 70, 96},{139, 16, 79, 68, 93},
{140, 17, 77, 67, 89}, {141, 19, 75, 67, 85}, {142, 21, 73, 67, 81}, {143, 23, 72, 68, 78}, {144, 25, 71, 70, 75}, {145, 27, 74, 75, 74}, {146, 30, 74, 78, 72}, {147, 33, 75, 82, 71},{148, 36, 77, 86, 71},{149, 37, 79, 89, 71},
{150, 38, 81, 93, 72}, {151, 38, 84, 96, 74}, {152, 38, 87, 99, 77}, {153, 37, 89,100, 81}, {154, 36, 92,100, 85}, {155, 33, 93,100, 89}, {156, 30, 95, 99, 93}, {157, 27, 95, 96, 96},{158, 25, 95, 93, 98},{159, 23, 93, 89,100},
{160, 21, 92, 86,100}, {161, 19, 89, 82,100}, {162, 18, 87, 78, 98}, {163, 18, 84, 75, 96}, {164, 18, 81, 73, 93}, {165, 19, 79, 71, 89}, {166, 21, 77, 71, 85}, {167, 23, 75, 71, 81},{168, 25, 74, 73, 77},{169, 27, 76, 78, 77},
{170, 30, 77, 81, 74}, {171, 33, 78, 86, 74}, {172, 36, 81, 90, 74}, {173, 36, 84, 93, 77}, {174, 36, 87, 96, 81}, {175, 33, 90, 97, 85}, {176, 30, 92, 96, 90}, {177, 27, 92, 93, 94},{178, 25, 92, 90, 96},{179, 23, 90, 86, 96},
{180, 21, 87, 81, 96}, {181, 20, 84, 78, 94}, {182, 21, 81, 75, 90}, {183, 23, 78, 74, 85}, {184, 25, 77, 75, 81}, {185, 27, 78, 80, 80}, {186, 30, 79, 84, 77}, {187, 33, 81, 88, 77},{188, 33, 84, 91, 80},{189, 33, 87, 93, 83},
{190, 30, 89, 93, 88}, {191, 27, 90, 91, 91}, {192, 25, 89, 88, 93}, {193, 23, 87, 84, 93}, {194, 23, 84, 80, 91}, {195, 23, 81, 78, 88}, {196, 25, 79, 78, 83}, {197, 25, 82, 82, 85},{198, 25, 84, 84, 88},{199, 25, 86, 86, 89},
{200, 27, 80, 82, 81}, {201, 27, 82, 84, 83}, {202, 27, 84, 86, 85}, {203, 27, 86, 88, 88}, {204, 27, 88, 90, 89}, {205, 30, 82, 86, 81}, {206, 30, 84, 88, 83}, {207, 30, 86, 90, 85},{208, 29, 00, 70, 64},{209, 27, 00, 68, 66},
{210, 26, 00, 66, 67}, {211, 24, 00, 65, 69}, {212, 22, 00, 63, 71}, {213, 20, 00, 61, 73}, {214, 18, 00, 59, 75}, {215, 17, 00, 57, 77}, {216, 15, 00, 56, 79}, {217,  0, 62, 54, 79},{218,  0, 60, 53, 77},{219,  0, 59, 51, 75},
{220,  0, 57, 49, 74}, {221,  0, 55, 47, 72}, {222,  0, 53, 46, 70}, {223,  0, 52, 44, 68}, {224,  0, 50, 42, 67}, {225,  0, 48, 41, 65}, {226,  0, 46, 39, 63}, {227,  0, 45, 37, 61},{228,  0, 43, 35, 60},{229,  0, 41, 34, 58},
{230,  0, 40, 32, 56}, {231,  0, 38, 30, 54}, {232, 13, 36, 29, 53}, {233, 15, 00, 29, 51}, {234, 17, 00, 30, 50}, {235, 18, 00, 32, 48}, {236, 20, 00, 34, 46}, {237, 22, 00, 36, 44},{238, 24, 00, 37, 43},{239, 26, 00, 39, 41},
{240, 27, 00, 41, 39}, {241, 29, 00, 43, 00}, {242,  0, 74, 53,105}, {243,  0, 76, 55,107}, {244,  0, 78, 57,109}, {245,  0, 80, 59,111}, {246,  0, 26,  1, 55}, {247,  0, 27,  3, 56},{248,  0, 29,  5, 58},{249,  0, 31,  7, 60},
{250,  0, 32,  8, 61}, {251,  0, 34, 10, 63}, {252,  0, 36, 12, 65}, {253,  0, 37, 13, 67}, {254,  0, 39, 15, 68}, {255,  0, 41, 17, 70}, {256,  0, 43, 19, 75}, {257,  0, 44, 20, 74},{258,  0, 46, 22, 75},{259,  0, 48, 24, 77},
{260,  0, 49, 25, 79}, {261,  0, 51, 27, 80}, {262,  0, 53, 29, 82}, {263,  0, 55, 30, 84}, {264,  0, 57, 32, 86}, {265,  0, 58, 34, 88}, {266,  0, 60, 35, 90}, {267,  0, 62, 37, 91},{268,  0, 64, 39, 93},{269,  0, 65, 40, 95}};
 

boolean rightEyeQuartAnim [4][95] = {
{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 
  1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 
  0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 
  0, 0, 0, 0, 0 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 
  0, 0, 1, 1, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 
  1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
  1, 1, 0, 1, 1 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 
  1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 
  1, 1, 0, 0, 0 }
};


  
