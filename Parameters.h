//Values below are just for initialising and will be changed when synth is initialised to current panel controls & EEPROM settings
byte midiChannel = MIDI_CHANNEL_OMNI;//(EEPROM)
String patchName = INITPATCHNAME;
boolean encCW = true;//This is to set the encoder to increment when turned CW - Settings Option
float oscWaveform = 0;
String Wavestr = "          "; //for Display
int octave1 = 0;
int unison = 0;
int octaveModSW =0;
float filterRes = 0;
float filterResstr = 0;
float waveMonoMulti = 0;
float osc1Detune = 0;
float osc1Detunestr = 0;
int pitchBendRange = 8;
int PitchBendLevel = 0;
int PitchBendLevelstr = 0; // for display
float modWheelDepth = 0.2f;
float keytrackingAmount = 0.5;//MIDI CC & settings option (EEPROM)
float glideSpeedstr = 0; // for display
float glideSpeed = 0;
float glideAmountstr = 0; // for display
float glideAmount = 0;
float glideMonoMulti = 0;
float glideDest = 0;
int returnvalue = 0;
float ADSRInvert = 0;
float ADSRInvertstr = 0;
float LfoDelay = 1;
float LfoDelaystr = 0; // for display
String StratusLFOWaveform = "                ";
String StratusSUBWaveform = "                ";
String StratusOSCWaveform = "                    ";
String StratusADSRMode = "                ";
String LfoMultiplierstr = "    ";
float LfoSlope = 0;
float LfoSlopestr = 0;
float filterCutoff = 12000;
float filterCutoffstr = 12000; // for display
float filterEnv = 0; // not required
float LfoDepth = 0;
float LfoDepthstr = 0; // for display
int octave2 = 0;
int sync = 0;
int PatchA = 0;
int PatchB = 0;
int PatchC = 0;
float LfoRate = 0;
float LfoRatestr = 0; //for display
float ampAttack = 1;
float ampAttackstr = 1;
float ampDecay = 1;
float ampDecaystr = 1;
float ampSustain = 1;
float ampSustainstr = 1; // for display
float ampRelease = 1;
float ampReleasestr = 1;
float LFOWaveform = 1;
float LfoDest = 1;
float LfoMonoMulti = 1;
float altWave = 1;
float buttonA = 0;
float buttonB = 0;
float buttonC = 0;
