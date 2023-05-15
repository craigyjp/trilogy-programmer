#include "SettingsService.h"

void settingsMIDICh(int index, const char *value);
void settingsKeyTracking(int index, const char *value);
void settingsPitchBend(int index, const char *value);
void settingsModWheelDepth(int index, const char *value);
void settingsEncoderDir(char * value);

int currentIndexMIDICh();
int currentIndexKeyTracking();
int currentIndexPitchBend();
int currentIndexModWheelDepth();
int currentIndexEncoderDir();


void settingsMIDICh(int index, const char *value) {
  if (strcmp(value, "ALL") == 0) {
    midiChannel = MIDI_CHANNEL_OMNI;
  } else {
    midiChannel = atoi(value);
  }
  storeMidiChannel(midiChannel);
}

void settingsKeyTracking(int index, const char *value) {
  if (strcmp(value, "None") == 0) keytrackingAmount = 0;
  if (strcmp(value, "Half") == 0)  keytrackingAmount =  0.5;
  if (strcmp(value, "Full") == 0) keytrackingAmount =  1.0;
  storeKeyTracking(keytrackingAmount);
}

void settingsPitchBend(int index, const char *value) {
  pitchBendRange = atoi(value);
  storePitchBendRange(pitchBendRange);
}

void settingsModWheelDepth(int index, const char *value) {
  modWheelDepth = atoi(value) / 10.0f;
  storeModWheelDepth(modWheelDepth);
}

void settingsEncoderDir(int index, const char *value) {
  if (strcmp(value, "Type 1") == 0) {
    encCW = true;
  } else {
    encCW =  false;
  }
  storeEncoderDir(encCW ? 1 : 0);
}

int currentIndexMIDICh() {
  return getMIDIChannel();
}

int currentIndexKeyTracking() {
  float value = getKeyTracking();
  if (value == 0) return 0;
  if (value == 0.5)  return 1;
  if (value == 1.0) return 2;
  return 0;
}

int currentIndexPitchBend() {
  return  getPitchBendRange() - 1;
}

int currentIndexModWheelDepth() {
  return (getModWheelDepth() * 10) - 1;
}

int currentIndexEncoderDir() {
  return getEncoderDir() ? 0 : 1;
}

// add settings to the circular buffer
void setUpSettings() {
  settings::append(settings::SettingsOption{ "MIDI Ch.", {"All", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "\0"}, settingsMIDICh, currentIndexMIDICh});
  settings::append(settings::SettingsOption{ "Key Tracking", {"None", "Half", "Full", "\0"}, settingsKeyTracking, currentIndexKeyTracking});
  settings::append(settings::SettingsOption{ "Pitch Bend", {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "\0"}, settingsPitchBend, currentIndexPitchBend});
  settings::append(settings::SettingsOption{ "MW Depth", {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "\0"}, settingsModWheelDepth, currentIndexModWheelDepth});
  settings::append(settings::SettingsOption{ "Encoder", {"Type 1", "Type 2", "\0"}, settingsEncoderDir, currentIndexEncoderDir});
}
