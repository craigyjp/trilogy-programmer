/*
  Crumar Trilogy MUX - Firmware Rev 1.2

  Includes code by:
    Dave Benn - Handling MUXs, a few other bits and original inspiration  https://www.notesandvolts.com/2019/01/teensy-synth-part-10-hardware.html
    ElectroTechnique - This is based on Simons Tsynth 3.6 version with all the audio code stripped out and the DEMUX added

  Arduino IDE
  Tools Settings:
  Board: "Teensy3.6"
  USB Type: "Serial + MIDI"
  CPU Speed: "180"
  Optimize: "Fastest"

  Performance Tests   CPU  Mem
  180Mhz Faster       81.6 44
  180Mhz Fastest      77.8 44
  180Mhz Fastest+PC   79.0 44
  180Mhz Fastest+LTO  76.7 44
  240MHz Fastest+LTO  55.9 44

  Additional libraries:
    Agileware CircularBuffer available in Arduino libraries manager
    Replacement files are in the Modified Libraries folder and need to be placed in the teensy Audio folder.
*/

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <MIDI.h>
//#include <USBHost_t36.h>
#include "MidiCC.h"
#include "Constants.h"
#include "Parameters.h"
#include "PatchMgr.h"
#include "HWControls.h"
#include "EepromMgr.h"
#include "Settings.h"

#define PARAMETER 0 //The main page for displaying the current patch and control (parameter) changes
#define RECALL 1 //Patches list
#define SAVE 2 //Save patch page
#define REINITIALISE 3 // Reinitialise message
#define PATCH 4 // Show current patch bypassing PARAMETER
#define PATCHNAMING 5 // Patch naming page
#define DELETE 6 //Delete patch page
#define DELETEMSG 7 //Delete patch message page
#define SETTINGS 8 //Settings page
#define SETTINGSVALUE 9 //Settings page

unsigned int state = PARAMETER;

#include "ST7735Display.h"

boolean cardStatus = false;
boolean firstPatchLoaded = false;

struct VoiceAndNote
{
  int note;
  long timeOn;
};

struct VoiceAndNote voices[NO_OF_VOICES] = {
  { -1, 0},
  { -1, 0},
  { -1, 0},
  { -1, 0},
  { -1, 0},
  { -1, 0}
};

int prevNote = 48;//This is for glide to use previous note to glide from
float previousMillis = millis(); //For MIDI Clk Sync

int count = 0;//For MIDI Clk Sync
int DelayForSH = 20;
int patchNo = 1;//Current patch no
int voiceToReturn = -1; //Initialise
long earliestTime = millis(); //For voice allocation - initialise to now

void setup()
{
  setupDisplay();
  setUpSettings();
  setupHardware();

  cardStatus = SD.begin(BUILTIN_SDCARD);
  if (cardStatus)
  {
    Serial.println("SD card is connected");
    //Get patch numbers and names from SD card
    loadPatches();
    if (patches.size() == 0)
    {
      //save an initialised patch to SD card
      savePatch("1", INITPATCH);
      loadPatches();
    }
  }
  else
  {
    Serial.println("SD card is not connected or unusable");
    reinitialiseToPanel();
    showPatchPage("No SD", "conn'd / usable");
  }

  //Read MIDI Channel from EEPROM
  midiChannel = getMIDIChannel();
  Serial.println("MIDI Ch:" + String(midiChannel) + " (0 is Omni On)");

//  volumePrevious = RE_READ; //Force volume control to be read and set to current

  //Read Key Tracking from EEPROM, this can be set individually by each patch.
  keytrackingAmount = getKeyTracking();

  //Read Pitch Bend Range from EEPROM
  pitchBendRange = getPitchBendRange();

  //Read Mod Wheel Depth from EEPROM
  modWheelDepth = getModWheelDepth();

  //Read Encoder Direction from EEPROM
  encCW = getEncoderDir();

//  recallPatch(patchNo); //Load first patch
}

void myNoteOn(byte channel, byte note, byte velocity)
{
  //Check for out of range notes
//  if (note + oscPitchA < 0 || note + oscPitchA > 127 || note + oscPitchB < 0 || note + oscPitchB > 127)
    return;

  if (unison == 0)
  {
    switch (getVoiceNo(-1))
    {
      case 1:
//        keytracking1.amplitude(note * DIV127 * keytrackingAmount);
        voices[0].note = note;
        voices[0].timeOn = millis();
        updateVoice1();
//        filterEnvelope1.noteOn();
//        ampEnvelope1.noteOn();
        voiceOn[0] = true;
//        if (glideSpeed > 0 && note != prevNote)
//        {
//          glide1.amplitude((prevNote - note) * DIV12);   //Set glide to previous note frequency (limited to 1 octave max)
//          glide1.amplitude(0, glideSpeed * GLIDEFACTOR); //glide to current note
//        }
        break;
      case 2:
//        keytracking2.amplitude(note * DIV127 * keytrackingAmount);
        voices[1].note = note;
        voices[1].timeOn = millis();
        updateVoice2();
//        filterEnvelope2.noteOn();
//        ampEnvelope2.noteOn();
        voiceOn[1] = true;
//        if (glideSpeed > 0 && note != prevNote)
//        {
//          glide2.amplitude((prevNote - note) * DIV12);   //Set glide to previous note frequency (limited to 1 octave max)
//          glide2.amplitude(0, glideSpeed * GLIDEFACTOR); //glide to current note
//        }
        break;
      case 3:
//        keytracking3.amplitude(note * DIV127 * keytrackingAmount);
        voices[2].note = note;
        voices[2].timeOn = millis();
        updateVoice3();
//        filterEnvelope3.noteOn();
//        ampEnvelope3.noteOn();
        voiceOn[2] = true;
//        if (glideSpeed > 0 && note != prevNote)
//        {
//          glide3.amplitude((prevNote - note) * DIV12);   //Set glide to previous note frequency (limited to 1 octave max)
//          glide3.amplitude(0, glideSpeed * GLIDEFACTOR); //glide to current note
//        }
        break;
      case 4:
//        keytracking4.amplitude(note * DIV127 * keytrackingAmount);
        voices[3].note = note;
        voices[3].timeOn = millis();
        updateVoice4();
//        filterEnvelope4.noteOn();
//        ampEnvelope4.noteOn();
        voiceOn[3] = true;
//        if (glideSpeed > 0 && note != prevNote)
//        {
//          glide4.amplitude((prevNote - note) * DIV12);   //Set glide to previous note frequency (limited to 1 octave max)
//          glide4.amplitude(0, glideSpeed * GLIDEFACTOR); //glide to current note
//        }
        break;
      case 5:
//        keytracking5.amplitude(note * DIV127 * keytrackingAmount);
        voices[4].note = note;
        voices[4].timeOn = millis();
        updateVoice5();
//        filterEnvelope5.noteOn();
//        ampEnvelope5.noteOn();
        voiceOn[4] = true;
//        if (glideSpeed > 0 && note != prevNote)
//        {
//          glide5.amplitude((prevNote - note) * DIV12);   //Set glide to previous note frequency (limited to 1 octave max)
//          glide5.amplitude(0, glideSpeed * GLIDEFACTOR); //glide to current note
//        }
        break;
      case 6:
//        keytracking6.amplitude(note * DIV127 * keytrackingAmount);
        voices[5].note = note;
        voices[5].timeOn = millis();
        updateVoice6();
//        filterEnvelope6.noteOn();
//        ampEnvelope6.noteOn();
        voiceOn[5] = true;
//        if (glideSpeed > 0 && note != prevNote)
//        {
//          glide6.amplitude((prevNote - note) * DIV12);   //Set glide to previous note frequency (limited to 1 octave max)
//          glide6.amplitude(0, glideSpeed * GLIDEFACTOR); //glide to current note
//        }
        break;
    }
  }
  else
  {

    //UNISON MODE
//    keytracking1.amplitude(note * DIV127 * keytrackingAmount);
//    keytracking2.amplitude(note * DIV127 * keytrackingAmount);
//    keytracking3.amplitude(note * DIV127 * keytrackingAmount);
//    keytracking4.amplitude(note * DIV127 * keytrackingAmount);
//    keytracking5.amplitude(note * DIV127 * keytrackingAmount);
//    keytracking6.amplitude(note * DIV127 * keytrackingAmount);
    voices[0].note = note;
    voices[0].timeOn = millis();
    updateVoice1();
    voices[1].note = note;
    voices[1].timeOn = millis();
    updateVoice2();
    voices[2].note = note;
    voices[2].timeOn = millis();
    updateVoice3();
    voices[3].note = note;
    voices[3].timeOn = millis();
    updateVoice4();
    voices[4].note = note;
    voices[4].timeOn = millis();
    updateVoice5();
    voices[5].note = note;
    voices[5].timeOn = millis();
    updateVoice6();

//    filterEnvelope1.noteOn();
//    filterEnvelope2.noteOn();
//    filterEnvelope3.noteOn();
//    filterEnvelope4.noteOn();
//    filterEnvelope5.noteOn();
//    filterEnvelope6.noteOn();
//
//    ampEnvelope1.noteOn();
//    ampEnvelope2.noteOn();
//    ampEnvelope3.noteOn();
//    ampEnvelope4.noteOn();
//    ampEnvelope5.noteOn();
//    ampEnvelope6.noteOn();

    voiceOn[0] = true;
    voiceOn[1] = true;
    voiceOn[2] = true;
    voiceOn[3] = true;
    voiceOn[4] = true;
    voiceOn[5] = true;

  }
}

void myNoteOff(byte channel, byte note, byte velocity)
{
  if (unison == 0)
  {
    switch (getVoiceNo(note))
    {
      case 1:
//        filterEnvelope1.noteOff();
//        ampEnvelope1.noteOff();
        prevNote = voices[0].note;
        voices[0].note = -1;
        voiceOn[0] = false;
        break;
      case 2:
//        filterEnvelope2.noteOff();
//        ampEnvelope2.noteOff();
        prevNote = voices[1].note;
        voices[1].note = -1;
        voiceOn[1] = false;
        break;
      case 3:
//        filterEnvelope3.noteOff();
//        ampEnvelope3.noteOff();
        prevNote = voices[2].note;
        voices[2].note = -1;
        voiceOn[2] = false;
        break;
      case 4:
//        filterEnvelope4.noteOff();
//        ampEnvelope4.noteOff();
        prevNote = voices[3].note;
        voices[3].note = -1;
        voiceOn[3] = false;
        break;
      case 5:
//        filterEnvelope5.noteOff();
//        ampEnvelope5.noteOff();
        prevNote = voices[4].note;
        voices[4].note = -1;
        voiceOn[4] = false;
        break;
      case 6:
//        filterEnvelope6.noteOff();
//        ampEnvelope6.noteOff();
        prevNote = voices[5].note;
        voices[5].note = -1;
        voiceOn[5] = false;
        break;
    }
  }
  else
  {
    //UNISON MODE
    //If statement prevents the previous different note
    //ending the current note when released
    if (voices[0].note == note)allNotesOff();
    prevNote = note;
  }
}

void allNotesOff()
{

  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH);
}

int getVoiceNo(int note)
{
  voiceToReturn = -1;      //Initialise
  earliestTime = millis(); //Initialise to now
  if (note == -1)
  {
    //NoteOn() - Get the oldest free voice (recent voices may be still on release stage)
    for (int i = 0; i < NO_OF_VOICES; i++)
    {
      if (voices[i].note == -1)
      {
        if (voices[i].timeOn < earliestTime)
        {
          earliestTime = voices[i].timeOn;
          voiceToReturn = i;
        }
      }
    }
    if (voiceToReturn == -1)
    {
      //No free voices, need to steal oldest sounding voice
      earliestTime = millis(); //Reinitialise
      for (int i = 0; i < NO_OF_VOICES; i++)
      {
        if (voices[i].timeOn < earliestTime)
        {
          earliestTime = voices[i].timeOn;
          voiceToReturn = i;
        }
      }
    }
    return voiceToReturn + 1;
  }
  else
  {
    //NoteOff() - Get voice number from note
    for (int i = 0; i < NO_OF_VOICES; i++)
    {
      if (voices[i].note == note)
      {
        return i + 1;
      }
    }
  }
  //Shouldn't get here, return voice 1
  return 1;
}

void updateVoice1()
{
}

void updateVoice2()
{
}

void updateVoice3()
{
}
void updateVoice4()
{
}

void updateVoice5()
{
}

void updateVoice6()
{
}

int getStratusLFOWaveform(int value)
{
  if (value >= 0 && value < 127)
  {
    StratusLFOWaveform = "Sawtooth Up";
  }
  else if (value >= 128 && value < 255)
  {
    StratusLFOWaveform = "Sawtooth Down";
  }
  else if (value >= 256 && value < 383)
  {
    StratusLFOWaveform = "Squarewave";
  }
  else if (value >= 384 && value < 511)
  {
    StratusLFOWaveform = "Triangle";
  }
  else if (value >= 512 && value < 639)
  {
    StratusLFOWaveform = "Sinewave";
  }
  else if (value >= 640 && value < 767)
  {
    StratusLFOWaveform = "Sweeps";
  }
  else if (value >= 768 && value < 895)
  {
    StratusLFOWaveform = "Lumps";
  }
  else
  {
    StratusLFOWaveform = "Sample & Hold";
  }
}

void updateoctave1()
{
  if (octave1 == 0)
  {
    showCurrentParameterPage("Octave 1", "Off");
    digitalWrite(OCTAVE1_LED, LOW);  // LED off
  }
  else
  {
    showCurrentParameterPage("Octave 1", "On");
    digitalWrite(OCTAVE1_LED, HIGH);  // LED on
  }
}

void updateoctave2()
{
  if (octave2 == 0)
  {
    showCurrentParameterPage("Octave 2", "Off");
    digitalWrite(OCTAVE2_LED, LOW);  // LED off
  }
  else
  {
    showCurrentParameterPage("Octave 2", "On");
    digitalWrite(OCTAVE2_LED, HIGH);  // LED on
  }
}

void updateglideSpeed()
{
  glideSpeedstr = (glideSpeed * 10);
  if (glideSpeedstr < 1000)
  {
    showCurrentParameterPage("Glide Speed", String(int(glideSpeedstr)) + " ms");
  }
  else
  {
    showCurrentParameterPage("Glide Speed", String(glideSpeedstr * 0.001) + " s");
  }
}

void updateMuxglideSpeed()
{
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, int(glideSpeed));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH);
}

void updatefilterRes()
{
  showCurrentParameterPage("Resonance", int(filterResstr));
}

void updateMuxfilterRes()
{
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, int(filterRes));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH);
}

void updateADSRInvert()
{
  showCurrentParameterPage("ADSR Polarity", ADSRInvertstr);
}

void updateMuxADSRInvert()
{
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, int(ADSRInvert));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH);
}

void updatesAllVoices() {
  updateVoice1();
  updateVoice2();
  updateVoice3();
  updateVoice4();
  updateVoice5();
  updateVoice6();
}

void updateLfoDelay()
{
  showCurrentParameterPage("LFO Delay", String(LfoDelaystr * 10) + " Seconds");
}

void updateMuxLfoDelay()
{
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, int(LfoDelay));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH);
}

void updatealtWave()
{
    if (altWave > 511 )
  {
    showCurrentParameterPage("Alt Sq/Sw", String("Off"));
  }
  else
  {
    showCurrentParameterPage("Alt Sq/Sw", String("On"));
  }
}

void updateMuxaltWave()
{
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (altWave));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
}

void updatewaveMonoMulti()
{
    if (waveMonoMulti > 511 )
  {
    showCurrentParameterPage("Wave Enable", String("Multi"));
  }
  else
  {
    showCurrentParameterPage("Wave Enable", String("Mono"));
  }
}

void updateMuxwaveMonoMulti()
{
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (waveMonoMulti));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
}

void updateQuickPatch()
{
  if (buttonA > 511)
  {
    PatchA = 1;
  }
  else
  {
    PatchA = 0;
  }
    if (buttonB > 511)
  {
    PatchB = 2;
  }
  else
  {
    PatchB = 0;
  }
    if (buttonC > 511)
  {
    PatchC = 4;
  }
  else
  {
    PatchC = 0;
  }
  patchNo = (PatchA + PatchB + PatchC + 1);
      state = PATCH;
  recallPatch(patchNo);
      state = PARAMETER;
}

void updateWaveformSaw()
{
  if (oscWaveform > 796)
  {
    Wavestr = "Sawtooth";
  }
  else if (oscWaveform <796 && oscWaveform > 255)
  {
    Wavestr = "Squarewave";
  } 
  else
  {
    Wavestr = "Mixed";
  }
  showCurrentParameterPage("Osc Waveform", (Wavestr));
}

void updateMuxWaveformSaw()
{
    if (oscWaveform > 796)
  {
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);

  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
  }
  else if (oscWaveform <796 && oscWaveform > 255)
  {
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);

  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
  } 
  else
  {
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);

  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
  }
}

void updateFilterCutoff()
{
  showCurrentParameterPage("Cutoff", String(filterCutoffstr) + " Hz");
}

void updateMuxFilterCutoff()
{
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_1, LOW); 
  analogWrite(A21, (filterCutoff));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH);
}

void updateosc1Detune()
{
  showCurrentParameterPage("OSC1 Detune", String(osc1Detunestr));
}

void updateMuxosc1Detune()
{
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_1, LOW); 
  analogWrite(A21, (osc1Detune));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH); 
}

void updateLfoSlope()
{
  showCurrentParameterPage("LFO Slope", int(LfoSlopestr));
}

void updateMuxLfoSlope()
{
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_1, LOW); 
  analogWrite(A21, (LfoSlope));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH); 
}

void updateKeyTracking()
{
  showCurrentParameterPage("Key Tracking", String(keytrackingAmount));
}

void updateLfoDepth()
{
  showCurrentParameterPage("LFO Depth", int(LfoDepthstr));
}

void updateMuxLfoDepth()
{
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, int(LfoDepth));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH);
}

void updateglideAmount()
{
  showCurrentParameterPage("Glide Amount", int(glideAmountstr));
}

void updateMuxglideAmount()
{
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, int(glideAmount));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH);
}

void updateLfoRate()
{
  showCurrentParameterPage("LFO Rate", String(LfoRatestr) + " Hz");
}

void updateMuxLfoRate()
{
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, int(LfoRate));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH);  
}

void updatesync()
{
  showCurrentParameterPage("Oscillator Sync", sync == 1 ? "On" : "Off");
  digitalWrite(SYNC_LED, sync == 1 ? HIGH : LOW);  // LED
}

void updateAttack()
{
  if (ampAttackstr < 1000)
  {
    showCurrentParameterPage("Attack", String(int(ampAttackstr)) + " ms", AMP_ENV);
  }
  else
  {
    showCurrentParameterPage("Attack", String(ampAttackstr * 0.001) + " s", AMP_ENV);
  }
}

void updateMuxAttack()
{
// MUX 1_0
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, (ampAttack));
  delayMicroseconds(DelayForSH);  
  digitalWrite(DEMUX_EN_1, HIGH);
}

void updateDecay()
{  
  if (ampDecaystr < 1000)
  {
    showCurrentParameterPage("Decay", String(int(ampDecaystr)) + " ms", AMP_ENV);
  }
  else
  {
    showCurrentParameterPage("Decay", String(ampDecaystr * 0.001) + " s", AMP_ENV);
  }
}

void updateMuxDecay()
{
// MUX 1_1
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, (ampDecay));
  delayMicroseconds(DelayForSH);  
  digitalWrite(DEMUX_EN_1, HIGH);
}

void updateSustain()
{
  showCurrentParameterPage("Sustain", String(ampSustainstr), AMP_ENV);
}

void updateMuxSustain()
{
// MUX 1_2
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, (ampSustain));
  delayMicroseconds(DelayForSH);  
  digitalWrite(DEMUX_EN_1, HIGH);
}

void updateRelease()
{
  if (ampReleasestr < 1000)
  {
    showCurrentParameterPage("Release", String(int(ampReleasestr)) + " ms", AMP_ENV);
  }
  else
  {
    showCurrentParameterPage("Release", String(ampReleasestr * 0.001) + " s", AMP_ENV);
  }
}

void updateMuxRelease()
{
// MUX 1_4
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, (ampRelease));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH);
}


void updateoctaveModSwitch()
{
  if (octaveModSW == 1)
  {
    showCurrentParameterPage("Octave Mod", "On");
    digitalWrite(OCTAVEMOD_LED, HIGH);  // LED on
  }
  else
  {
    showCurrentParameterPage("Octave Mod", "Off");
    digitalWrite(OCTAVEMOD_LED, LOW);  // LED off
  }
}

void updateStratusLFOWaveform()
{
  getStratusLFOWaveform(LFOWaveform);
  showCurrentParameterPage("LFO", StratusLFOWaveform);
}

void updateMuxStratusLFOWaveform()
{
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_1, LOW);
  analogWrite(A21, int(LFOWaveform));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_1, HIGH);
}

void updateLfoDest()
{
  if (LfoDest > 750)
  {
    showCurrentParameterPage("LFO Routing", String("VCO"));
  }
  else if (LfoDest < 749 && LfoDest > 250)
  {
    showCurrentParameterPage("LFO Routing", String("VCF"));
  }
  else
  {
    showCurrentParameterPage("LFO Routing", String("VCA"));
  }
}

void updateMuxLfoDest()
{
  if (LfoDest > 750)
  {
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);

  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);

  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
  }
  else if (LfoDest < 749 && LfoDest > 250)
  {
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);

  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);

  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
  }
  else
  {
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);

  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);

  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
  }
}

void updateglideDest()
{
  if (glideDest > 796)
  {
    showCurrentParameterPage("Glide Direction", String("Osc1 & 2 Down"));
  }
  else if (glideDest < 796 && glideDest > 512)
  {
    showCurrentParameterPage("Glide Direction", String("Osc2 Down"));
  }
    else if (glideDest < 511 && glideDest > 256)
  {
    showCurrentParameterPage("Glide Direction", String("Osc2 Up"));
  }
  else
  {
    showCurrentParameterPage("Glide Direction", String("Osc1 & 2 Up"));
  }
}

void updateMuxglideDest()
{
//  Serial.print("MUX Glide value ");
//  Serial.println(glideDest);
  if (glideDest >= 800)
  {
//A
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
//B
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
//C
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
//D
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH); 
  }
  else if (glideDest < 700 && glideDest > 600)
  {
//A
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
//B
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
//C
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
//D
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
  }
      else if (glideDest < 400 && glideDest > 300)
  {
//A
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
//B
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
//C
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
//D
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
  }
  else
  {
  //A
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
//B
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
//C
  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (0));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
//D
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (1023));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
  }
}

void updateLfoMonoMulti()
{
    if (LfoMonoMulti > 511 )
  {
    showCurrentParameterPage("LFO Reset", String("Multi"));
  }
  else
  {
    showCurrentParameterPage("LFO Reset", String("Mono"));
  }
}

void updateMuxLfoMonoMulti()
{
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, LOW);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (LfoMonoMulti));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
}

void updateglideMonoMulti()
{
    if (glideMonoMulti > 511 )
  {
    showCurrentParameterPage("Glide Enable", String("Multi"));
  }
  else
  {
    showCurrentParameterPage("Glide Enable", String("Mono"));
  }
}

void updateMuxglideMonoMulti()
{
  digitalWrite(DEMUX_0, HIGH);
  digitalWrite(DEMUX_1, HIGH);
  digitalWrite(DEMUX_2, HIGH);
  digitalWrite(DEMUX_3, HIGH);
  digitalWrite(DEMUX_EN_2, LOW);
  analogWrite(A21, (glideMonoMulti));
  delayMicroseconds(DelayForSH);
  digitalWrite(DEMUX_EN_2, HIGH);
}


void updatePatchname()
{
  showPatchPage(String(patchNo), patchName);
}

void myControlChange(byte channel, byte control, int value)
{
  switch (control)
  {

//MUX1
    case CCampattack:
      ampAttack = value;
      ampAttackstr = ENVTIMES[value / 8];
      updateAttack();
      break;

    case CCampdecay:
      ampDecay = value;
      ampDecaystr = ENVTIMES[value / 8];
      updateDecay();
      break;

    case CCampsustain:
      ampSustain = value;
      ampSustainstr = LINEAR_FILTERMIXERSTR[value / 8];
      updateSustain();
      break;

    case CCADSRInvert:
      ADSRInvertstr = LINEARCENTREZERO[value / 8];
      ADSRInvert = value;
      updateADSRInvert();
      break;
      
    case CCamprelease:
      ampRelease = value;
      ampReleasestr = ENVTIMES[value / 8];
      updateRelease();
      break;
 
    case CCfilterCutoff:
      filterCutoff = value;
      filterCutoffstr = FILTERCUTOFF[value / 8];
      updateFilterCutoff();
      break;

    case CCfilterRes:
      filterRes = value;
      filterResstr = int(value / 8);
      updatefilterRes();
      break;

    case CCLfoRate:
      LfoRatestr = LINEAR_NORMAL [value / 8]; // for display
      LfoRate = value;
      updateLfoRate();
      break;
      
    case CCLFOWaveform:
      LFOWaveform = value;
      updateStratusLFOWaveform();
      break;

    case CCLfoSlope:
      LfoSlope = value;
      LfoSlopestr = LINEAR_NORMAL[value / 8];
      updateLfoSlope();
      break;
      
    case CCLfoDelay:
      LfoDelaystr = LINEAR[value / 8];
      LfoDelay = value;
      updateLfoDelay();
      break;
      
    case CCLfoDepth:
      LfoDepth = value;
      LfoDepthstr = LINEAR_NORMAL[value/8]; // for display
      updateLfoDepth();
      break;

    case CCglideSpeed:
      glideSpeedstr = LINEAR_NORMAL[value / 8];
      glideSpeed = value;
      updateglideSpeed();
      break;

      case CCglideAmount:
      glideAmountstr = LINEAR_NORMAL[value / 8]; // for display
      glideAmount = value;
      updateglideAmount();
      break;
      
    case CCosc1Detune:
      osc1Detunestr = PITCH_DETUNE[value / 8];
      osc1Detune = value;
      updateosc1Detune();
      break;
      
//MUX2

    case CCoscWaveform:
      oscWaveform = value;
      updateWaveformSaw();
      break;

    case CCaltWave:
      altWave = value;
      updatealtWave();
      break;

    case CCwaveMonoMulti:
      waveMonoMulti = value;
      updatewaveMonoMulti();
      break;

    case CCglideMonoMulti:
      glideMonoMulti = value;
      updateglideMonoMulti();
      break;

    case CCglideDest:
      glideDest = value;
      updateglideDest();
      break;

    case CCButtonA:
      buttonA = value;
      updateQuickPatch();
      break;
      
    case CCButtonB:
      buttonB = value;
      updateQuickPatch();
      break;

    case CCButtonC:
      buttonC = value;
      updateQuickPatch();
      break;
      
    case CCLfoMonoMulti:
      LfoMonoMulti = value;
      updateLfoMonoMulti();
      break;

    case CCLfoDest:
      LfoDest = value;
      updateLfoDest();
      break;

//SWITCHES

    case CCoctaveModSW:
      value > 0 ? octaveModSW = 1 : octaveModSW = 0;
      updateoctaveModSwitch();
      break;

    case CCoctave1:
      value > 0 ? octave1 = 1 : octave1 = 0;
      updateoctave1();
      break;

    case CCoctave2:
      value > 0 ? octave2 = 1 : octave2 = 0;
      updateoctave2();
      break;

    case CCsync:
      value > 0 ? sync = 1 : sync = 0;
      updatesync();
      break;
                 
    case CCallnotesoff:
      allNotesOff();
      break;
  }
}

void myProgramChange(byte channel, byte program)
{
  state = PATCH;
  patchNo = program + 1;
  recallPatch(patchNo);
  Serial.print("MIDI Pgm Change:");
  Serial.println(patchNo);
  state = PARAMETER;
}

void recallPatch(int patchNo)
{
  allNotesOff();
  File patchFile = SD.open(String(patchNo).c_str());
  if (!patchFile)
  {
    Serial.println("File not found");
  }
  else
  {
    String data[NO_OF_PARAMS]; //Array of data read in
    recallPatchData(patchFile, data);
    setCurrentPatchData(data);
    patchFile.close();
  }
}

void setCurrentPatchData(String data[])
{
  patchName = data[0];
  glideAmount = data[1].toFloat();
  oscWaveform = data[2].toFloat();
  octave1 = data[3].toInt();
  octaveModSW = data[4].toInt();
  filterRes = data[5].toFloat();
  glideSpeed = data[6].toFloat();
  waveMonoMulti = data[7].toFloat();
  glideDest = data[8].toFloat();
  glideMonoMulti = data[9].toFloat();
  ADSRInvert = data[10].toFloat();
  LfoDelay = data[11].toFloat();
  filterCutoff = data[12].toFloat();
  LfoDepth = data[13].toFloat();
  octave2 = data[14].toInt();
  sync = data[15].toInt();
  LfoRate = data[16].toFloat();
  ampAttack = data[17].toFloat();
  ampDecay = data[18].toFloat();
  ampSustain = data[19].toFloat();
  ampRelease = data[20].toFloat();
  LFOWaveform = data[21].toFloat();
  LfoDest = data[22].toFloat();
  LfoMonoMulti = data[23].toFloat();
  altWave = data[24].toFloat();
  osc1Detune = data[25].toFloat();
  LfoSlope = data[26].toFloat();

//MUX1
  updateAttack();
  updateDecay();
  updateSustain();
  updateADSRInvert();
  updateRelease();
  updateFilterCutoff();
  updatefilterRes();
  updateLfoRate();
  updateStratusLFOWaveform();
  updateLfoSlope();
  updateLfoDelay();
  updateLfoDepth();
  updateglideSpeed();
  updateglideAmount();
  updateosc1Detune();

//MUX2
  updateWaveformSaw();
  updatealtWave();
  updatewaveMonoMulti();
  updateglideMonoMulti();
  updateglideDest();
  updateLfoMonoMulti();
  updateLfoDest();
  
//Switches
  updateoctave1();
  updateoctave2();
  updatesync();
  updateoctaveModSwitch();

//Patchname
  updatePatchname();
  
//  Serial.print("Set Patch: ");
//  Serial.println(patchName);
}

String getCurrentPatchData()
{
  return patchName + "," + String(glideAmount) + "," + String(oscWaveform) + "," + String(octave1) + "," + String(octaveModSW) + "," + String(filterRes) + "," + String(glideSpeed) + "," + String(waveMonoMulti) + "," +
         String(glideDest) + "," + String(glideMonoMulti) + "," + String(ADSRInvert) + "," + String(LfoDelay) + "," + String(filterCutoff) + "," + String(LfoDepth) + "," + String(octave2) + "," +
         String(sync) + ","  + String(LfoRate) + "," + String(ampAttack) + "," + String(ampDecay) + "," + String(ampSustain) + "," + String(ampRelease) + "," +
         String(LFOWaveform) + "," + String(LfoDest) + "," + String(LfoMonoMulti) + "," + String(altWave) + "," + String(osc1Detune) + "," + String(LfoSlope);
}

void checkMux()
{

  mux1Read = adc->adc1->analogRead(MUX1_S);
  mux2Read = adc->adc1->analogRead(MUX2_S);
  if (mux1Read > (mux1ValuesPrev[muxInput] + QUANTISE_FACTOR) || mux1Read < (mux1ValuesPrev[muxInput] - QUANTISE_FACTOR))
  {
    mux1ValuesPrev[muxInput] = mux1Read;
    //mux1Read = (mux1Read >> 3); //Change range to 0-127

    switch (muxInput)
    {
      case MUX1_attack:
        myControlChange(midiChannel, CCampattack, mux1Read);
        break;
      case MUX1_decay:
        myControlChange(midiChannel, CCampdecay, mux1Read);
        break;
      case MUX1_sustain:
        myControlChange(midiChannel, CCampsustain, mux1Read);
        break;
      case MUX1_ADSRInvert:
        myControlChange(midiChannel, CCADSRInvert, mux1Read);
        break;
      case MUX1_release:
        myControlChange(midiChannel, CCamprelease, mux1Read);
        break;
      case MUX1_filterCutoff:
        myControlChange(midiChannel, CCfilterCutoff, mux1Read);
        break;
      case MUX1_filterRes:
        myControlChange(midiChannel, CCfilterRes, mux1Read);
        break;
      case MUX1_LfoRate:
        myControlChange(midiChannel, CCLfoRate, mux1Read);
        break;
      case MUX1_LfoWaveform:
        myControlChange(midiChannel, CCLFOWaveform, mux1Read);
        break;
      case MUX1_LfoSlope:
        myControlChange(midiChannel, CCLfoSlope, mux1Read);
        break;
      case MUX1_LfoDelay:
        myControlChange(midiChannel, CCLfoDelay, mux1Read);
        break;
      case MUX1_LfoDepth:
        myControlChange(midiChannel, CCLfoDepth, mux1Read);
        break;
      case MUX1_glideSpeed:
        myControlChange(midiChannel, CCglideSpeed, mux1Read);
        break;
      case MUX1_glideAmount:
        myControlChange(midiChannel, CCglideAmount, mux1Read);
        break;
//      case MUX1_14:
//        //Pick-up
//        myControlChange(midiChannel, CCmux1_14, mux1Read);
//        break;
      case MUX1_osc1Detune:
        myControlChange(midiChannel, CCosc1Detune, mux1Read);
        break;
    }
  }

  if (mux2Read > (mux2ValuesPrev[muxInput] + QUANTISE_FACTOR) || mux2Read < (mux2ValuesPrev[muxInput] - QUANTISE_FACTOR))
  {
    mux2ValuesPrev[muxInput] = mux2Read;
    //mux2Read = (mux2Read >> 3); //Change range to 0-127

    switch (muxInput)
    {
      case MUX2_oscWaveform:
        myControlChange(midiChannel, CCoscWaveform, mux2Read);
        break;
//      case MUX2_2:
//        myControlChange(midiChannel, CCmux2_1, mux2Read);
//        break;
      case MUX2_altWave:
        myControlChange(midiChannel, CCaltWave, mux2Read);
        break;
      case MUX2_waveMonoMulti:
        myControlChange(midiChannel, CCwaveMonoMulti, mux2Read);
        break;
//      case MUX2_4:
//        //Pick-up
//        myControlChange(midiChannel, CCmux2_4, mux2Read);
//        break;
//      case MUX2_5:
//        myControlChange(midiChannel, CCmux2_5, mux2Read);
//        break;
//      case MUX2_6:
//        //Pick-up
//        myControlChange(midiChannel, CCmux2_6, mux2Read);
//        break;
      case MUX2_glideMonoMulti:
        myControlChange(midiChannel, CCglideMonoMulti, mux2Read);
        break;
      case MUX2_glideDest:
        myControlChange(midiChannel, CCglideDest, mux2Read);
        break;
//      case MUX2_9:
//        myControlChange(midiChannel, CCmux2_9, mux2Read);
//        break;
//      case MUX2_10:
//        myControlChange(midiChannel, CCmux2_10, mux2Read);
//        break;
      case MUX2_ButtonA:
        myControlChange(midiChannel, CCButtonA, mux2Read);
        break;
      case MUX2_ButtonB:
        myControlChange(midiChannel, CCButtonB, mux2Read);
        break;
      case MUX2_ButtonC:
        myControlChange(midiChannel, CCButtonC, mux2Read);
        break;
      case MUX2_LfoMonoMulti:
        myControlChange(midiChannel, CCLfoMonoMulti, mux2Read);
        break;
      case MUX2_LfoDest:
        myControlChange(midiChannel, CCLfoDest, mux2Read);
        break;
    }
  }

  muxInput++;
  if (muxInput >= MUXCHANNELS)
    muxInput = 0;
      if (!firstPatchLoaded) {
      recallPatch(patchNo); //Load first patch after all controls read
      firstPatchLoaded = true;
      }
  digitalWriteFast(MUX_0, muxInput & B0001);
  digitalWriteFast(MUX_1, muxInput & B0010);
  digitalWriteFast(MUX_2, muxInput & B0100);
  digitalWriteFast(MUX_3, muxInput & B1000);
//  delayMicroseconds(500);
}

void updateDemux()
{  
  // Demux 1 
    updateMuxAttack();
    updateMuxDecay();
    updateMuxSustain();
    updateMuxADSRInvert();
    updateMuxRelease();
    updateMuxFilterCutoff();
    updateMuxfilterRes();
    updateMuxLfoRate(); 
    updateMuxStratusLFOWaveform();
    updateMuxLfoSlope();
    updateMuxLfoDelay();  
    updateMuxLfoDepth(); 
    updateMuxglideSpeed();
    updateMuxglideAmount();
//spare    
    updateMuxosc1Detune();

  // Demux 2

  updateMuxWaveformSaw();
//  spare
  updateMuxaltWave();
  updateMuxwaveMonoMulti();
//spare
//spare
//spare  
  updateMuxglideMonoMulti();
  updateMuxglideDest();
//spare
//spare
//spare
//spare
//sapre  
  updateMuxLfoMonoMulti();
  updateMuxLfoDest();
}

void checkSwitches()
{
  octave1Switch.update();
  if (octave1Switch.fallingEdge())
  {
    octave1 = !octave1;
    myControlChange(midiChannel, CCoctave1, octave1);
  }

  octavemodSwitch.update();
  if (octavemodSwitch.fallingEdge())
  {
    octaveModSW = !octaveModSW;
    myControlChange(midiChannel, CCoctaveModSW, octaveModSW);
  }

  syncSwitch.update();
  if (syncSwitch.fallingEdge())
  {
    sync = !sync;
    myControlChange(midiChannel, CCsync, sync);
  }

  octave2Switch.update();
  if (octave2Switch.fallingEdge())
  {
    octave2 = !octave2;
    myControlChange(midiChannel, CCoctave2, octave2);
  }

  saveButton.update();
  if (saveButton.read() == LOW && saveButton.duration() > HOLD_DURATION)
  {
    switch (state)
    {
      case PARAMETER:
      case PATCH:
        state = DELETE;
        saveButton.write(HIGH); //Come out of this state
        del = true;             //Hack
        break;
    }
  }
  else if (saveButton.risingEdge())
  {
    if (!del)
    {
      switch (state)
      {
        case PARAMETER:
          if (patches.size() < PATCHES_LIMIT)
          {
            resetPatchesOrdering(); //Reset order of patches from first patch
            patches.push({patches.size() + 1, INITPATCHNAME});
            state = SAVE;
          }
          break;
        case SAVE:
          //Save as new patch with INITIALPATCH name or overwrite existing keeping name - bypassing patch renaming
          patchName = patches.last().patchName;
          state = PATCH;
          savePatch(String(patches.last().patchNo).c_str(), getCurrentPatchData());
          showPatchPage(patches.last().patchNo, patches.last().patchName);
          patchNo = patches.last().patchNo;
          loadPatches(); //Get rid of pushed patch if it wasn't saved
          setPatchesOrdering(patchNo);
          renamedPatch = "";
          state = PARAMETER;
          break;
        case PATCHNAMING:
          if (renamedPatch.length() > 0) patchName = renamedPatch;//Prevent empty strings
          state = PATCH;
          savePatch(String(patches.last().patchNo).c_str(), getCurrentPatchData());
          showPatchPage(patches.last().patchNo, patchName);
          patchNo = patches.last().patchNo;
          loadPatches(); //Get rid of pushed patch if it wasn't saved
          setPatchesOrdering(patchNo);
          renamedPatch = "";
          state = PARAMETER;
          break;
      }
    }
    else
    {
      del = false;
    }
  }

  settingsButton.update();
  if (settingsButton.read() == LOW && settingsButton.duration() > HOLD_DURATION)
  {
    //If recall held, set current patch to match current hardware state
    //Reinitialise all hardware values to force them to be re-read if different
    state = REINITIALISE;
    reinitialiseToPanel();
    settingsButton.write(HIGH); //Come out of this state
    reini = true;           //Hack
  }
  else if (settingsButton.risingEdge())
  { //cannot be fallingEdge because holding button won't work
    if (!reini)
    {
      switch (state)
      {
        case PARAMETER:
          settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
          state = SETTINGS;
          break;
        case SETTINGS:
          settingsOptions.push(settingsOptions.shift());
          settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
        case SETTINGSVALUE:
          //Same as pushing Recall - store current settings item and go back to options
          settingsHandler(settingsOptions.first().value[settingsValueIndex], settingsOptions.first().handler);
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
          state = SETTINGS;
          break;
      }
    }
    else
    {
      reini = false;
    }
  }

  backButton.update();
  if (backButton.read() == LOW && backButton.duration() > HOLD_DURATION)
  {
    //If Back button held, Panic - all notes off
    allNotesOff();
    backButton.write(HIGH); //Come out of this state
    panic = true;           //Hack
  }
  else if (backButton.risingEdge())
  { //cannot be fallingEdge because holding button won't work
    if (!panic)
    {
      switch (state)
      {
        case RECALL:
          setPatchesOrdering(patchNo);
          state = PARAMETER;
          break;
        case SAVE:
          renamedPatch = "";
          state = PARAMETER;
          loadPatches();//Remove patch that was to be saved
          setPatchesOrdering(patchNo);
          break;
        case PATCHNAMING:
          charIndex = 0;
          renamedPatch = "";
          state = SAVE;
          break;
        case DELETE:
          setPatchesOrdering(patchNo);
          state = PARAMETER;
          break;
        case SETTINGS:
          state = PARAMETER;
          break;
        case SETTINGSVALUE:
          settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
          state = SETTINGS;
          break;
      }
    }
    else
    {
      panic = false;
    }
  }

  //Encoder switch
  recallButton.update();
  if (recallButton.read() == LOW && recallButton.duration() > HOLD_DURATION)
  {
    //If Recall button held, return to current patch setting
    //which clears any changes made
    state = PATCH;
    //Recall the current patch
    patchNo = patches.first().patchNo;
    recallPatch(patchNo);
    state = PARAMETER;
    recallButton.write(HIGH); //Come out of this state
    recall = true;            //Hack
  }
  else if (recallButton.risingEdge())
  {
    if (!recall)
    {
      switch (state)
      {
        case PARAMETER:
          state = RECALL;//show patch list
          break;
        case RECALL:
          state = PATCH;
          //Recall the current patch
          patchNo = patches.first().patchNo;
          recallPatch(patchNo);
          state = PARAMETER;
          break;
        case SAVE:
          showRenamingPage(patches.last().patchName);
          patchName  = patches.last().patchName;
          state = PATCHNAMING;
          break;
        case PATCHNAMING:
          if (renamedPatch.length() < 13)
          {
            renamedPatch.concat(String(currentCharacter));
            charIndex = 0;
            currentCharacter = CHARACTERS[charIndex];
            showRenamingPage(renamedPatch);
          }
          break;
        case DELETE:
          //Don't delete final patch
          if (patches.size() > 1)
          {
            state = DELETEMSG;
            patchNo = patches.first().patchNo;//PatchNo to delete from SD card
            patches.shift();//Remove patch from circular buffer
            deletePatch(String(patchNo).c_str());//Delete from SD card
            loadPatches();//Repopulate circular buffer to start from lowest Patch No
            renumberPatchesOnSD();
            loadPatches();//Repopulate circular buffer again after delete
            patchNo = patches.first().patchNo;//Go back to 1
            recallPatch(patchNo);//Load first patch
          }
          state = PARAMETER;
          break;
        case SETTINGS:
          //Choose this option and allow value choice
          settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGSVALUE);
          state = SETTINGSVALUE;
          break;
        case SETTINGSVALUE:
          //Store current settings item and go back to options
          settingsHandler(settingsOptions.first().value[settingsValueIndex], settingsOptions.first().handler);
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
          state = SETTINGS;
          break;
      }
    }
    else
    {
      recall = false;
    }
  }
}

void reinitialiseToPanel()
{
  //This sets the current patch to be the same as the current hardware panel state - all the pots
  //The four button controls stay the same state
  //This reinialises the previous hardware values to force a re-read
  muxInput = 0;
  for (int i = 0; i < MUXCHANNELS; i++)
  {
    mux1ValuesPrev[i] = RE_READ;
    mux2ValuesPrev[i] = RE_READ;
  }
  patchName = INITPATCHNAME;
  showPatchPage("Initial", "Panel Settings");
}

void checkEncoder()
{
  //Encoder works with relative inc and dec values
  //Detent encoder goes up in 4 steps, hence +/-3

  long encRead = encoder.read();
  if ((encCW && encRead > encPrevious + 3) || (!encCW && encRead < encPrevious - 3) )
  {
    switch (state)
    {
      case PARAMETER:
        state = PATCH;
        patches.push(patches.shift());
        patchNo = patches.first().patchNo;
        recallPatch(patchNo);
        state = PARAMETER;
        break;
      case RECALL:
        patches.push(patches.shift());
        break;
      case SAVE:
        patches.push(patches.shift());
        break;
      case PATCHNAMING:
        if (charIndex == TOTALCHARS) charIndex = 0;//Wrap around
        currentCharacter = CHARACTERS[charIndex++];
        showRenamingPage(renamedPatch + currentCharacter);
        break;
      case DELETE:
        patches.push(patches.shift());
        break;
      case SETTINGS:
        settingsOptions.push(settingsOptions.shift());
        settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
        showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex] , SETTINGS);
        break;
      case SETTINGSVALUE:
        if (settingsOptions.first().value[settingsValueIndex + 1] != '\0')
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[++settingsValueIndex], SETTINGSVALUE);
        break;
    }
    encPrevious = encRead;
  }
  else if ((encCW && encRead < encPrevious - 3) || (!encCW && encRead > encPrevious + 3))
  {
    switch (state)
    {
      case PARAMETER:
        state = PATCH;
        patches.unshift(patches.pop());
        patchNo = patches.first().patchNo;
        recallPatch(patchNo);
        state = PARAMETER;
        break;
      case RECALL:
        patches.unshift(patches.pop());
        break;
      case SAVE:
        patches.unshift(patches.pop());
        break;
      case PATCHNAMING:
        if (charIndex == -1)
          charIndex = TOTALCHARS - 1;
        currentCharacter = CHARACTERS[charIndex--];
        showRenamingPage(renamedPatch + currentCharacter);
        break;
      case DELETE:
        patches.unshift(patches.pop());
        break;
      case SETTINGS:
        settingsOptions.unshift(settingsOptions.pop());
        settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
        showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
        break;
      case SETTINGSVALUE:
        if (settingsValueIndex > 0)
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[--settingsValueIndex], SETTINGSVALUE);
        break;
    }
    encPrevious = encRead;
  }
}

void loop()
{
//  myusb.Task();
//  midi1.read(midiChannel);   //USB HOST MIDI Class Compliant
//  usbMIDI.read(midiChannel); //USB Client MIDI
//  MIDI.read(midiChannel);    //MIDI 5 Pin DIN
  
  checkMux();
  updateDemux();
  checkSwitches();
  checkEncoder();
}
