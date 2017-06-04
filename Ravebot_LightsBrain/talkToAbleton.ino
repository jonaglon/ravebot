bool newBeat=false;

void playRandomTune(int genre)
{
  int newTrackNumber = 0;
  do
    newTrackNumber = random(10);
  while (newTrackNumber == currentTrack);  
  
  playTune(genre, newTrackNumber);
}

void playTune(int genre, int track) {
  
  // send stuff to ableton to start the new track  
  stopAllAbletonTracks(); 
  start16BeatAbletonTrack(); // start the midi track in ableton which sends midi time codes back here
  setAbletonTempo(tunesLibrary[genre][track].bpm);
  PlayAbletonTrack(genre+1, track);

  // change the current track in this program
  currentBar = 0;
  currentGenre = genre;
  currentTrack = track;
  currentBpm=tunesLibrary[genre][track].bpm;
  inTheMix=false;

  // tell the other arduino what you're doing
  sendSerialToMega(2,(genre*100)+track);

  // choose the track to mix in to
  chooseNextTrack();
}

void PlayAbletonTrack(int channel, int trackNumber)
{
  if ((channel < 1) || (trackNumber > 127))
    return;
   
  channel=channel+175;
  sendMidi(channel, trackNumber, 127);
}

void stopAllAbletonTracks()
{
  sendMidi(176, 125, 127);
}

void start16BeatAbletonTrack()
{
  sendMidi(176, 126, 127);
}

void setMainVolume(int volume)  // 0 - 127
{
  if (volume > 127)
    volume = 127;
  sendMidi(176, 127, volume);
}

void setAbletonTempo(int tempo)  // 80 - 207 bpm only
{
    sendMidi(177, 125, tempo-80);
}

void setRobotVolume(int volume)  // 0 - 127
{
  sendMidi(177, 126, volume);
}

void setCrossfader(int value)  // 0 - 127
{
  sendMidi(177, 127, value);
}

//  plays a MIDI note.  Doesn't check to see that
//  cmd is greater than 127, or that data values are  less than 127:
// 176 is channel=1, 190 channel=15
void sendMidi(int channel, int trackNumber, int velocity)
{
  Serial.write(channel);
  Serial.write(trackNumber);
  Serial.write(velocity);
}

void sendMidiTest(int channel, int trackNumber, int velocity)
{
  Serial.print("channel:");
  //Serial.write(channel);
  Serial.print(channel);
  Serial.println(" ");
  Serial.print("track:");
  //Serial.write(trackNumber);
  Serial.print(trackNumber);
  Serial.println(" ");
  Serial.print("velocity:");
  //Serial.write(velocity);
  Serial.print(velocity);
  Serial.println(" ");
}



