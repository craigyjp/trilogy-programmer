/*
  Crumar Trilogy MUX

  Includes code by:
    Dave Benn - Handling MUXs, a few other bits and original inspiration  https://www.notesandvolts.com/2019/01/teensy-synth-part-10-hardware.html
    ElectroTechnique - This is based on Simons Tsynth 3.6 version with all the audio code stripped out and the DEMUX added

  Arduino IDE
  Tools Settings:
  Board: "Teensy3.6"
  USB Type: "Serial + MIDI"
  CPU Speed: "180"
  Optimize: "Fastest"

  Additional libraries:
    Agileware CircularBuffer available in Arduino libraries manager
    Replacement files are in the Modified Libraries folder and need to be placed in the teensy Audio folder.
*/

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <MIDI.h>
#include "MidiCC.h"
#include "Constants.h"
#include "Parameters.h"
#include "PatchMgr.h"
#include "HWControls.h"
#include "EepromMgr.h"
#include "Settings.h"

#define PARAMETER 0      //The main page for displaying the current patch and control (parameter) changes
#define RECALL 1         //Patches list
#define SAVE 2           //Save patch page
#define REINITIALISE 3   // Reinitialise message
#define PATCH 4          // Show current patch bypassing PARAMETER
#define PATCHNAMING 5    // Patch naming page
#define DELETE 6         //Delete patch page
#define DELETEMSG 7      //Delete patch message page
#define SETTINGS 8       //Settings page
#define SETTINGSVALUE 9  //Settings page

unsigned int state = PARAMETER;

#include "ST7735Display.h"

boolean cardStatus = false;
int DelayForSH3 = 50;
int patchNo = 1;               //Current patch no

void setup() {
  setupDisplay();
  setUpSettings();
  setupHardware();

  cardStatus = SD.begin(BUILTIN_SDCARD);
  if (cardStatus) {
    Serial.println("SD card is connected");
    //Get patch numbers and names from SD card
    loadPatches();
    if (patches.size() == 0) {
      //save an initialised patch to SD card
      savePatch("1", INITPATCH);
      loadPatches();
    }
  } else {
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
  patchNo = getLastPatch();
  recallPatch(patchNo);  //Load first patch
}

void allNotesOff() {
  ampRelease = 0;
}

void updateoctave1() {
  if (octave1 == 0) {
    showCurrentParameterPage("Octave 1", "Off");
    digitalWrite(OCTAVE1_LED, LOW);  // LED off
  } else {
    showCurrentParameterPage("Octave 1", "On");
    digitalWrite(OCTAVE1_LED, HIGH);  // LED on
  }
}

void updateoctave2() {
  if (octave2 == 0) {
    showCurrentParameterPage("Octave 2", "Off");
    digitalWrite(OCTAVE2_LED, LOW);  // LED off
  } else {
    showCurrentParameterPage("Octave 2", "On");
    digitalWrite(OCTAVE2_LED, HIGH);  // LED on
  }
}

void updateglideSpeed() {
  glideSpeedstr = (glideSpeed * 10);
  if (glideSpeedstr < 1000) {
    showCurrentParameterPage("Glide Speed", String(int(glideSpeedstr)) + " ms");
  } else {
    showCurrentParameterPage("Glide Speed", String(glideSpeedstr * 0.001) + " s");
  }
}

void updatefilterRes() {
  showCurrentParameterPage("Resonance", int(filterResstr));
}

void updateorgan16() {
  showCurrentParameterPage("Organ 16'", int(displaystr));
}

void updateorgan8() {
  showCurrentParameterPage("Organ 8'", int(displaystr));
}

void updateorgan4() {
  showCurrentParameterPage("Organ 4'", int(displaystr));
}

void updateorgan2() {
  showCurrentParameterPage("Organ 2'", int(displaystr));
}

void updatesynthVolume() {
  showCurrentParameterPage("Synth Volume", int(displaystr));
}

void updateorganVolume() {
  showCurrentParameterPage("Organ Volume", int(displaystr));
}

void updatestringsVolume() {
  showCurrentParameterPage("String Volume", int(displaystr));
}

void updatetimbre() {
  showCurrentParameterPage("String Timbre", int(timbre / 8));
}

void updatefootages() {
  foot8str = foot8str * 2;
  if (foot8str >= 128) {
    foot8str = 127;
  }
  foot16str = foot16str * 2;
  if (foot16str >= 128) {
    foot16str = 127;
  }
  showCurrentParameterPage("Foootages", "   " + String(foot16str, 0) + " - " + String(foot8str, 0));
}

void updatestringAttack() {
  showCurrentParameterPage("String Attack", int(stringAttack / 8));
}

void updatestringRelease() {
  showCurrentParameterPage("String Release", int(stringRelease / 8));
}

void updateADSRInvert() {
  showCurrentParameterPage("ADSR Polarity", ADSRInvertstr);
}

void updateLfoDelay() {
  showCurrentParameterPage("LFO Delay", String(LfoDelaystr * 10) + " Seconds");
}

void updatealtWave() {
  if (altWave > 511) {
    showCurrentParameterPage("Alt Sq/Sw", String("Off"));
  } else {
    showCurrentParameterPage("Alt Sq/Sw", String("On"));
  }
}

void updatewaveMonoMulti() {
  if (waveMonoMulti > 511) {
    showCurrentParameterPage("Wave Enable", String("Multi"));
  } else {
    showCurrentParameterPage("Wave Enable", String("Mono"));
  }
}

void updateQuickPatch() {
  if (buttonA > 511) {
    PatchA = 1;
  } else {
    PatchA = 0;
  }
  if (buttonB > 511) {
    PatchB = 2;
  } else {
    PatchB = 0;
  }
  if (buttonC > 511) {
    PatchC = 4;
  } else {
    PatchC = 0;
  }
  patchNo = (PatchA + PatchB + PatchC + 1);
  state = PATCH;
  recallPatch(patchNo);
  state = PARAMETER;
}

void updateWaveformSaw() {
  if (oscWaveform > 796) {
    Wavestr = "Sawtooth";
    sawtooth = 1023;
    squarewave = 0;
  } else if (oscWaveform < 796 && oscWaveform > 255) {
    Wavestr = "Squarewave";
    sawtooth = 0;
    squarewave = 1023;
  } else {
    Wavestr = "Mixed";
    sawtooth = 1023;
    squarewave = 1023;
  }
  showCurrentParameterPage("Osc Waveform", (Wavestr));
}

void updateFilterCutoff() {
  showCurrentParameterPage("Cutoff", String(filterCutoffstr) + " Hz");
}

void updateosc1Detune() {
  showCurrentParameterPage("OSC1 Detune", String(osc1Detunestr));
}

void updateLfoSlope() {
  showCurrentParameterPage("LFO Slope", int(LfoSlopestr));
}

void updateKeyTracking() {
  showCurrentParameterPage("Key Tracking", String(keytrackingAmount));
}

void updateLfoDepth() {
  showCurrentParameterPage("LFO Depth", int(LfoDepthstr));
}

void updateglideAmount() {
  showCurrentParameterPage("Glide Amount", int(glideAmountstr));
}

void updateLfoRate() {
  showCurrentParameterPage("LFO Rate", String(LfoRatestr) + " Hz");
}

void updatesync() {
  showCurrentParameterPage("Oscillator Sync", sync == 1 ? "On" : "Off");
  digitalWrite(SYNC_LED, sync == 1 ? HIGH : LOW);  // LED
}

void updateAttack() {
  if (ampAttackstr < 1000) {
    showCurrentParameterPage("Attack", String(int(ampAttackstr)) + " ms", AMP_ENV);
  } else {
    showCurrentParameterPage("Attack", String(ampAttackstr * 0.001) + " s", AMP_ENV);
  }
}

void updateDecay() {
  if (ampDecaystr < 1000) {
    showCurrentParameterPage("Decay", String(int(ampDecaystr)) + " ms", AMP_ENV);
  } else {
    showCurrentParameterPage("Decay", String(ampDecaystr * 0.001) + " s", AMP_ENV);
  }
}

void updateSustain() {
  showCurrentParameterPage("Sustain", String(ampSustainstr), AMP_ENV);
}

void updateRelease() {
  if (ampReleasestr < 1000) {
    showCurrentParameterPage("Release", String(int(ampReleasestr)) + " ms", AMP_ENV);
  } else {
    showCurrentParameterPage("Release", String(ampReleasestr * 0.001) + " s", AMP_ENV);
  }
}

void updateoctaveModSwitch() {
  if (octaveModSW == 1) {
    showCurrentParameterPage("Octave Mod", "On");
    digitalWrite(OCTAVEMOD_LED, HIGH);  // LED on
  } else {
    showCurrentParameterPage("Octave Mod", "Off");
    digitalWrite(OCTAVEMOD_LED, LOW);  // LED off
  }
}

void updateStratusLFOWaveform() {
  switch (LFOWaveformstr) {
    case 0:
      StratusLFOWaveform = "Sawtooth Up";
      break;

    case 1:
      StratusLFOWaveform = "Sawtooth Down";
      break;

    case 2:
      StratusLFOWaveform = "Squarewave";
      break;

    case 3:
      StratusLFOWaveform = "Triangle";
      break;

    case 4:
      StratusLFOWaveform = "Sinewave";
      break;

    case 5:
      StratusLFOWaveform = "Sweeps";
      break;

    case 6:
      StratusLFOWaveform = "Lumps";
      break;

    case 7:
      StratusLFOWaveform = "Sample & Hold";
      break;
  }
  showCurrentParameterPage("LFO", StratusLFOWaveform);
}

void updateLfoDest() {
  if (LfoDest > 750) {
    showCurrentParameterPage("LFO Routing", String("VCO"));
    LfoVCO = 1023;
    LfoVCF = 0;
    LfoVCA = 0;
  } else if (LfoDest < 749 && LfoDest > 250) {
    showCurrentParameterPage("LFO Routing", String("VCF"));
    LfoVCO = 0;
    LfoVCF = 1023;
    LfoVCA = 0;
  } else {
    showCurrentParameterPage("LFO Routing", String("VCA"));
    LfoVCO = 0;
    LfoVCF = 0;
    LfoVCA = 1023;
  }
}

void updateglideDest() {
  if (glideDest > 796) {
    showCurrentParameterPage("Glide Direction", String("Osc1 & 2 Down"));
    glideA = 1023;
    glideB = 0;
    glideC = 1023;
    glideD = 0;
  } else if (glideDest < 796 && glideDest > 512) {
    showCurrentParameterPage("Glide Direction", String("Osc2 Down"));
    glideA = 0;
    glideB = 0;
    glideC = 1023;
    glideD = 0;
  } else if (glideDest < 511 && glideDest > 256) {
    showCurrentParameterPage("Glide Direction", String("Osc2 Up"));
    glideA = 0;
    glideB = 1023;
    glideC = 0;
    glideD = 0;
  } else {
    showCurrentParameterPage("Glide Direction", String("Osc1 & 2 Up"));
    glideA = 0;
    glideB = 1023;
    glideC = 0;
    glideD = 1023;
  }
}

void updateLfoMonoMulti() {
  if (LfoMonoMulti > 511) {
    showCurrentParameterPage("LFO Reset", String("Multi"));
  } else {
    showCurrentParameterPage("LFO Reset", String("Mono"));
  }
}

void updateglideMonoMulti() {
  if (glideMonoMulti > 511) {
    showCurrentParameterPage("Glide Enable", String("Multi"));
  } else {
    showCurrentParameterPage("Glide Enable", String("Mono"));
  }
}

void updatePatchname() {
  showPatchPage(String(patchNo), patchName);
}

void myControlChange(byte channel, byte control, int value) {
  switch (control) {

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
      LfoRatestr = LINEAR_NORMAL[value / 8];  // for display
      LfoRate = value;
      updateLfoRate();
      break;

    case CCLFOWaveform:
      LFOWaveform = value;
      LFOWaveformstr = value >> 7;
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
      LfoDepthstr = LINEAR_NORMAL[value / 8];  // for display
      updateLfoDepth();
      break;

    case CCglideSpeed:
      glideSpeedstr = LINEAR_NORMAL[value / 8];
      glideSpeed = value;
      updateglideSpeed();
      break;

    case CCglideAmount:
      glideAmountstr = LINEAR_NORMAL[value / 8];  // for display
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

      //MUX3

    case CCorgan16:
      organ16 = value;
      displaystr = value / 8;
      updateorgan16();
      break;

    case CCorgan8:
      organ8 = value;
      displaystr = value / 8;
      updateorgan8();
      break;

    case CCorgan4:
      organ4 = value;
      displaystr = value / 8;
      updateorgan4();
      break;

    case CCorgan2:
      organ2 = value;
      displaystr = value / 8;
      updateorgan2();
      break;

    case CCsynthVolume:
      synthVolume = value;
      displaystr = value / 8;
      updatesynthVolume();
      break;

    case CCorganVolume:
      organVolume = value;
      displaystr = value / 8;
      updateorganVolume();
      break;

    case CCstringsVolume:
      stringsVolume = value;
      displaystr = value / 8;
      updatestringsVolume();
      break;

    case CCtimbre:
      timbre = value;
      updatetimbre();
      break;

    case CCfootages:
      foot8str = ROTARY[value / 8];
      foot16str = LINEAR_INVERSE[value / 8];
      foot8 = NEWSTRING8[value / 8];
      foot16 = NEWSTRING16[value / 8];
      updatefootages();
      break;

    case CCstringAttack:
      stringAttack = value;
      updatestringAttack();
      break;

    case CCstringRelease:
      stringRelease = value;
      updatestringRelease();
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

void myProgramChange(byte channel, byte program) {
  state = PATCH;
  patchNo = program + 1;
  recallPatch(patchNo);
  Serial.print("MIDI Pgm Change:");
  Serial.println(patchNo);
  state = PARAMETER;
}

void recallPatch(int patchNo) {
  allNotesOff();
  File patchFile = SD.open(String(patchNo).c_str());
  if (!patchFile) {
    Serial.println("File not found");
  } else {
    String data[NO_OF_PARAMS];  //Array of data read in
    recallPatchData(patchFile, data);
    setCurrentPatchData(data);
    patchFile.close();
    storeLastPatch(patchNo);
  }
}

void setCurrentPatchData(String data[]) {
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
  sawtooth = data[27].toFloat();
  squarewave = data[28].toFloat();
  LfoVCO = data[29].toFloat();
  LfoVCF = data[30].toFloat();
  LfoVCA = data[31].toFloat();
  glideA = data[32].toFloat();
  glideB = data[33].toFloat();
  glideC = data[34].toFloat();
  glideD = data[35].toFloat();
  organ16 = data[36].toFloat();
  organ8 = data[37].toFloat();
  organ4 = data[38].toFloat();
  organ2 = data[39].toFloat();
  synthVolume = data[40].toFloat();
  organVolume = data[41].toFloat();
  stringsVolume = data[42].toFloat();
  timbre = data[43].toFloat();
  footages = data[44].toFloat();
  foot16 = data[45].toFloat();
  foot8 = data[46].toFloat();
  stringAttack = data[47].toFloat();
  stringRelease = data[48].toFloat();


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

String getCurrentPatchData() {
  return patchName + "," + String(glideAmount) + "," + String(oscWaveform) + "," + String(octave1) + "," + String(octaveModSW) + "," + String(filterRes) + "," + String(glideSpeed) + "," + String(waveMonoMulti) + "," + String(glideDest) + "," + String(glideMonoMulti) + "," + String(ADSRInvert) + "," + String(LfoDelay) + "," + String(filterCutoff) + "," + String(LfoDepth) + "," + String(octave2) + "," + String(sync) + "," + String(LfoRate) + "," + String(ampAttack) + "," + String(ampDecay) + "," + String(ampSustain) + "," + String(ampRelease) + "," + String(LFOWaveform) + "," + String(LfoDest) + "," + String(LfoMonoMulti) + "," + String(altWave) + "," + String(osc1Detune) + "," + String(LfoSlope) + "," + String(sawtooth) + "," + String(squarewave) + "," + String(LfoVCO) + "," + String(LfoVCF) + "," + String(LfoVCA) + "," + String(glideA) + "," + String(glideB) + "," + String(glideC) + "," + String(glideD) + "," + String(organ16) + "," + String(organ8) + "," + String(organ4) + "," + String(organ2) + "," + String(synthVolume) + "," + String(organVolume) + "," + String(stringsVolume) + "," + String(timbre) + "," + String(footages) + "," + String(foot16) + "," + String(foot8) + "," + String(stringAttack) + "," + String(stringRelease);
}

void checkMux() {

  mux1Read = adc->adc1->analogRead(MUX1_S);
  mux2Read = adc->adc1->analogRead(MUX2_S);
  mux3Read = adc->adc1->analogRead(MUX3_S);

  if (mux1Read > (mux1ValuesPrev[muxInput] + QUANTISE_FACTOR) || mux1Read < (mux1ValuesPrev[muxInput] - QUANTISE_FACTOR)) {
    mux1ValuesPrev[muxInput] = mux1Read;
    //mux1Read = (mux1Read >> 3); //Change range to 0-127

    switch (muxInput) {
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

  if (mux2Read > (mux2ValuesPrev[muxInput] + QUANTISE_FACTOR) || mux2Read < (mux2ValuesPrev[muxInput] - QUANTISE_FACTOR)) {
    mux2ValuesPrev[muxInput] = mux2Read;
    //mux2Read = (mux2Read >> 3); //Change range to 0-127

    switch (muxInput) {
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

  if (mux3Read > (mux3ValuesPrev[muxInput] + QUANTISE_FACTOR) || mux3Read < (mux3ValuesPrev[muxInput] - QUANTISE_FACTOR)) {
    mux3ValuesPrev[muxInput] = mux3Read;
    //mux3Read = (mux3Read >> 3); //Change range to 0-127

    switch (muxInput) {
      case MUX3_organ16:
        myControlChange(midiChannel, CCorgan16, mux3Read);
        break;
      case MUX3_organ8:
        myControlChange(midiChannel, CCorgan8, mux3Read);
        break;
      case MUX3_organ4:
        myControlChange(midiChannel, CCorgan4, mux3Read);
        break;
      case MUX3_organ2:
        myControlChange(midiChannel, CCorgan2, mux3Read);
        break;
      case MUX3_synthVolume:
        myControlChange(midiChannel, CCsynthVolume, mux3Read);
        break;
      case MUX3_organVolume:
        myControlChange(midiChannel, CCorganVolume, mux3Read);
        break;
      case MUX3_stringsVolume:
        myControlChange(midiChannel, CCstringsVolume, mux3Read);
        break;
      case MUX3_timbre:
        myControlChange(midiChannel, CCtimbre, mux3Read);
        break;
      case MUX3_footages:
        myControlChange(midiChannel, CCfootages, mux3Read);
        break;
      case MUX3_stringAttack:
        myControlChange(midiChannel, CCstringAttack, mux3Read);
        break;
      case MUX3_stringRelease:
        myControlChange(midiChannel, CCstringRelease, mux3Read);
        break;
        //      case MUX3_11:
        //        myControlChange(midiChannel, CCButtonA, mux3Read);
        //        break;
        //      case MUX3_12:
        //        myControlChange(midiChannel, CCButtonB, mux3Read);
        //        break;
        //      case MUX3_13:
        //        myControlChange(midiChannel, CCButtonC, mux3Read);
        //        break;
        //      case MUX3_14:
        //        myControlChange(midiChannel, CCLfoMonoMulti, mux3Read);
        //        break;
        //      case MUX3_15:
        //        myControlChange(midiChannel, CCLfoDest, mux3Read);
        //        break;
    }
  }

  muxInput++;
  if (muxInput >= MUXCHANNELS)
    muxInput = 0;

  digitalWriteFast(MUX_0, muxInput & B0001);
  digitalWriteFast(MUX_1, muxInput & B0010);
  digitalWriteFast(MUX_2, muxInput & B0100);
  digitalWriteFast(MUX_3, muxInput & B1000);
}

void writeDemux() {

  //DEMUX 1

  switch (muxOutput) {
    case 0:
      analogWrite(A21, int(ampAttack));
      break;
    case 1:
      analogWrite(A21, int(ampDecay));
      break;
    case 2:
      analogWrite(A21, int(ampSustain));
      break;
    case 3:
      analogWrite(A21, int(ADSRInvert));
      break;
    case 4:
      analogWrite(A21, int(ampRelease));
      break;
    case 5:
      analogWrite(A21, int(filterCutoff));
      break;
    case 6:
      analogWrite(A21, int(filterRes));
      break;
    case 7:
      analogWrite(A21, int(LfoRate));
      break;
    case 8:
      analogWrite(A21, int(LFOWaveform));
      break;
    case 9:
      analogWrite(A21, int(LfoSlope));
      break;
    case 10:
      analogWrite(A21, int(LfoDelay));
      break;
    case 11:
      analogWrite(A21, int(LfoDepth));
      break;
    case 12:
      analogWrite(A21, int(glideSpeed));
      break;
    case 13:
      analogWrite(A21, int(glideAmount));
      break;
    case 15:
      analogWrite(A21, int(osc1Detune));
      break;
  }
  digitalWriteFast(DEMUX_EN_1, LOW);
  delayMicroseconds(DelayForSH3);
  digitalWriteFast(DEMUX_EN_1, HIGH);

  //DEMUX 2


  switch (muxOutput) {
    case 0:
      analogWrite(A21, int(sawtooth));
      break;
    case 1:
      analogWrite(A21, int(squarewave));
      break;
    case 2:
      analogWrite(A21, int(altWave));
      break;
    case 3:
      analogWrite(A21, int(waveMonoMulti));
      break;
    case 4:
      analogWrite(A21, int(LfoVCO));
      break;
    case 5:
      analogWrite(A21, int(LfoVCF));
      break;
    case 6:
      analogWrite(A21, int(LfoVCA));
      break;
    case 7:
      analogWrite(A21, int(LfoMonoMulti));
      break;
    case 8:
      analogWrite(A21, int(glideA));
      break;
    case 9:
      analogWrite(A21, int(glideB));
      break;
    case 10:
      analogWrite(A21, int(glideC));
      break;
    case 11:
      analogWrite(A21, int(glideD));
      break;
    case 15:
      analogWrite(A21, int(glideMonoMulti));
      break;
  }
  digitalWriteFast(DEMUX_EN_2, LOW);
  delayMicroseconds(DelayForSH3);
  digitalWriteFast(DEMUX_EN_2, HIGH);

  //DEMUX 3


  switch (muxOutput) {
    case 0:
      analogWrite(A21, int(organ16 / 1.65));
      break;
    case 1:
      analogWrite(A21, int(organ8 / 1.65));
      break;
    case 2:
      analogWrite(A21, int(organ4 / 1.65));
      break;
    case 3:
      analogWrite(A21, int(organ2 / 1.65));
      break;
    case 4:
      analogWrite(A21, int(synthVolume / 1.65));
      break;
    case 5:
      analogWrite(A21, int(organVolume / 1.65));
      break;
    case 6:
      analogWrite(A21, int(stringsVolume / 1.65));
      break;
    case 7:
      analogWrite(A21, int(timbre / 1.65));
      break;
    case 8:
      analogWrite(A21, int((foot8 * 8) / 1.65));
      break;
    case 9:
      analogWrite(A21, int((foot16 * 8) / 1.65));
      break;
    case 10:
      analogWrite(A21, int(stringAttack));
      break;
    case 11:
      analogWrite(A21, int(stringRelease));
      break;
  }
  digitalWriteFast(DEMUX_EN_3, LOW);
  delayMicroseconds(DelayForSH3);
  digitalWriteFast(DEMUX_EN_3, HIGH);

  //  delayMicroseconds(DelayForSH3);
  muxOutput++;
  if (muxOutput >= DEMUXCHANNELS)
    muxOutput = 0;

  digitalWriteFast(DEMUX_0, muxOutput & B0001);
  digitalWriteFast(DEMUX_1, muxOutput & B0010);
  digitalWriteFast(DEMUX_2, muxOutput & B0100);
  digitalWriteFast(DEMUX_3, muxOutput & B1000);
}

void showSettingsPage() {
  showSettingsPage(settings::current_setting(), settings::current_setting_value(), state);
}

void checkSwitches() {
  octave1Switch.update();
  if (octave1Switch.numClicks() == 1) {
    octave1 = !octave1;
    myControlChange(midiChannel, CCoctave1, octave1);
  }

  octavemodSwitch.update();
  if (octavemodSwitch.numClicks() == 1) {
    octaveModSW = !octaveModSW;
    myControlChange(midiChannel, CCoctaveModSW, octaveModSW);
  }

  syncSwitch.update();
  if (syncSwitch.numClicks() == 1) {
    sync = !sync;
    myControlChange(midiChannel, CCsync, sync);
  }

  octave2Switch.update();
  if (octave2Switch.numClicks() == 1) {
    octave2 = !octave2;
    myControlChange(midiChannel, CCoctave2, octave2);
  }

  saveButton.update();
  if (saveButton.held()) {
    switch (state) {
      case PARAMETER:
      case PATCH:
        state = DELETE;
        break;
    }
  } else if (saveButton.numClicks() == 1) {
    switch (state) {
      case PARAMETER:
        if (patches.size() < PATCHES_LIMIT) {
          resetPatchesOrdering();  //Reset order of patches from first patch
          patches.push({ patches.size() + 1, INITPATCHNAME });
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
        loadPatches();  //Get rid of pushed patch if it wasn't saved
        setPatchesOrdering(patchNo);
        renamedPatch = "";
        state = PARAMETER;
        break;
      case PATCHNAMING:
        if (renamedPatch.length() > 0) patchName = renamedPatch;  //Prevent empty strings
        state = PATCH;
        savePatch(String(patches.last().patchNo).c_str(), getCurrentPatchData());
        showPatchPage(patches.last().patchNo, patchName);
        patchNo = patches.last().patchNo;
        loadPatches();  //Get rid of pushed patch if it wasn't saved
        setPatchesOrdering(patchNo);
        renamedPatch = "";
        state = PARAMETER;
        break;
    }
  }

  settingsButton.update();
  if (settingsButton.held()) {
    //If recall held, set current patch to match current hardware state
    //Reinitialise all hardware values to force them to be re-read if different
    state = REINITIALISE;
    reinitialiseToPanel();
  } else if (settingsButton.numClicks() == 1) {
    switch (state) {
      case PARAMETER:
        state = SETTINGS;
        showSettingsPage();
        break;
      case SETTINGS:
        showSettingsPage();
      case SETTINGSVALUE:
        settings::save_current_value();
        state = SETTINGS;
        showSettingsPage();
        break;
    }
  }

  backButton.update();
  if (backButton.held()) {
    //If Back button held, Panic - all notes off
    allNotesOff();                           //Come out of this state
  } else if (backButton.numClicks() == 1) {  //cannot be fallingEdge because holding button won't work
    switch (state) {
      case RECALL:
        setPatchesOrdering(patchNo);
        state = PARAMETER;
        break;
      case SAVE:
        renamedPatch = "";
        state = PARAMETER;
        loadPatches();  //Remove patch that was to be saved
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
        state = SETTINGS;
        showSettingsPage();
        break;
    }
  }

  //Encoder switch
  recallButton.update();
  if (recallButton.held()) {
    //If Recall button held, return to current patch setting
    //which clears any changes made
    state = PATCH;
    //Recall the current patch
    patchNo = patches.first().patchNo;
    recallPatch(patchNo);
    state = PARAMETER;
  } else if (recallButton.numClicks() == 1) {
    switch (state) {
      case PARAMETER:
        state = RECALL;  //show patch list
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
        patchName = patches.last().patchName;
        state = PATCHNAMING;
        break;
      case PATCHNAMING:
        if (renamedPatch.length() < 13) {
          renamedPatch.concat(String(currentCharacter));
          charIndex = 0;
          currentCharacter = CHARACTERS[charIndex];
          showRenamingPage(renamedPatch);
        }
        break;
      case DELETE:
        //Don't delete final patch
        if (patches.size() > 1) {
          state = DELETEMSG;
          patchNo = patches.first().patchNo;     //PatchNo to delete from SD card
          patches.shift();                       //Remove patch from circular buffer
          deletePatch(String(patchNo).c_str());  //Delete from SD card
          loadPatches();                         //Repopulate circular buffer to start from lowest Patch No
          renumberPatchesOnSD();
          loadPatches();                      //Repopulate circular buffer again after delete
          patchNo = patches.first().patchNo;  //Go back to 1
          recallPatch(patchNo);               //Load first patch
        }
        state = PARAMETER;
        break;
      case SETTINGS:
        state = SETTINGSVALUE;
        showSettingsPage();
        break;
      case SETTINGSVALUE:
        settings::save_current_value();
        state = SETTINGS;
        showSettingsPage();
        break;
    }
  }
}

void reinitialiseToPanel() {
  //This sets the current patch to be the same as the current hardware panel state - all the pots
  //The four button controls stay the same state
  //This reinialises the previous hardware values to force a re-read
  muxInput = 0;
  for (int i = 0; i < MUXCHANNELS; i++) {
    mux1ValuesPrev[i] = RE_READ;
    mux2ValuesPrev[i] = RE_READ;
    mux3ValuesPrev[i] = RE_READ;
  }
  patchName = INITPATCHNAME;
  showPatchPage("Initial", "Panel Settings");
}

void checkEncoder() {
  //Encoder works with relative inc and dec values
  //Detent encoder goes up in 4 steps, hence +/-3

  long encRead = encoder.read();
  if ((encCW && encRead > encPrevious + 3) || (!encCW && encRead < encPrevious - 3)) {
    switch (state) {
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
        if (charIndex == TOTALCHARS) charIndex = 0;  //Wrap around
        currentCharacter = CHARACTERS[charIndex++];
        showRenamingPage(renamedPatch + currentCharacter);
        break;
      case DELETE:
        patches.push(patches.shift());
        break;
      case SETTINGS:
        settings::increment_setting();
        showSettingsPage();
        break;
      case SETTINGSVALUE:
        settings::increment_setting_value();
        showSettingsPage();
        break;
    }
    encPrevious = encRead;
  } else if ((encCW && encRead < encPrevious - 3) || (!encCW && encRead > encPrevious + 3)) {
    switch (state) {
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
        settings::decrement_setting();
        showSettingsPage();
        break;
      case SETTINGSVALUE:
        settings::decrement_setting_value();
        showSettingsPage();
        break;
    }
    encPrevious = encRead;
  }
}

void loop() {
  //  myusb.Task();
  //  midi1.read(midiChannel);   //USB HOST MIDI Class Compliant
  //  usbMIDI.read(midiChannel); //USB Client MIDI
  //  MIDI.read(midiChannel);    //MIDI 5 Pin DIN

  checkMux();
  writeDemux();
  checkSwitches();
  checkEncoder();
}
