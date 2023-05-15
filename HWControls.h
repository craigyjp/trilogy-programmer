// This optional setting causes Encoder to use more optimized code,
// It must be defined before Encoder.h is included.
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include "TButton.h"
#include <ADC.h>
#include <ADC_util.h>

ADC *adc = new ADC();

#define MUX1_S A19 // pin 38
#define MUX2_S A22 // DAC1
#define MUX3_S A10 // A10

//Teensy 3.6 - Mux Pins
#define MUX_0 28
#define MUX_1 27
#define MUX_2 26
#define MUX_3 25

#define DEMUX_0 18
#define DEMUX_1 19
#define DEMUX_2 23
#define DEMUX_3 22

#define DEMUX_EN_1 6
#define DEMUX_EN_2 7
#define DEMUX_EN_3 9

//Mux 1 Connections
#define MUX1_attack 0
#define MUX1_decay 1
#define MUX1_sustain 2
#define MUX1_ADSRInvert 3
#define MUX1_release 4
#define MUX1_filterCutoff 5
#define MUX1_filterRes 6
#define MUX1_LfoRate 7
#define MUX1_LfoWaveform 8
#define MUX1_LfoSlope 9
#define MUX1_LfoDelay 10
#define MUX1_LfoDepth 11
#define MUX1_glideSpeed 12
#define MUX1_glideAmount 13
#define MUX1_14 14
#define MUX1_osc1Detune 15

//Mux 2 Connections
#define MUX2_oscWaveform 0
#define MUX2_2 1
#define MUX2_altWave 2
#define MUX2_waveMonoMulti 3
#define MUX2_4 4
#define MUX2_5 5
#define MUX2_6 6
#define MUX2_glideMonoMulti 7
#define MUX2_glideDest 8
#define MUX2_9 9
#define MUX2_10 10
#define MUX2_ButtonA 11
#define MUX2_ButtonB 12
#define MUX2_ButtonC 13
#define MUX2_LfoMonoMulti 14
#define MUX2_LfoDest 15

//Mux 3 Connections
#define MUX3_organ16 0
#define MUX3_organ8 1
#define MUX3_organ4 2
#define MUX3_organ2 3
#define MUX3_synthVolume 4
#define MUX3_organVolume 5
#define MUX3_stringsVolume 6
#define MUX3_timbre 7
#define MUX3_footages 8
#define MUX3_stringAttack 9
#define MUX3_stringRelease 10
#define MUX3_11 11
#define MUX3_12 12
#define MUX3_13 13
#define MUX3_14 14
#define MUX3_15 15

//Teensy 3.6 Pins
#define OCTAVEMOD_SW 39
#define SYNC_SW 30
#define OCTAVE1_SW 36
#define OCTAVE2_SW 16
#define RECALL_SW 17
#define SAVE_SW 24
#define SETTINGS_SW 12
#define BACK_SW 10

#define ENCODER_PINA 4
#define ENCODER_PINB 5

#define SYNC_LED 34
#define OCTAVE2_LED 35
#define OCTAVE1_LED 37
#define OCTAVEMOD_LED 14

#define MUXCHANNELS 16
#define DEMUXCHANNELS 16
#define QUANTISE_FACTOR 10

#define DEBOUNCE 30

static byte muxInput = 0;
static byte muxOutput = 0;
static int mux1ValuesPrev[MUXCHANNELS] = {};
static int mux2ValuesPrev[MUXCHANNELS] = {};
static int mux3ValuesPrev[MUXCHANNELS] = {};

static int mux1Read = 0;
static int mux2Read = 0;
static int mux3Read = 0;

static long encPrevious = 0;

//These are pushbuttons and require debouncing
TButton octavemodSwitch{OCTAVEMOD_SW, LOW, HOLD_DURATION, DEBOUNCE, CLICK_DURATION};
TButton syncSwitch{SYNC_SW, LOW, HOLD_DURATION, DEBOUNCE, CLICK_DURATION};
TButton octave1Switch{OCTAVE1_SW, LOW, HOLD_DURATION, DEBOUNCE, CLICK_DURATION};
TButton octave2Switch{OCTAVE2_SW, LOW, HOLD_DURATION, DEBOUNCE, CLICK_DURATION};

TButton recallButton{RECALL_SW, LOW, HOLD_DURATION, DEBOUNCE, CLICK_DURATION};
TButton saveButton{SAVE_SW, LOW, HOLD_DURATION, DEBOUNCE, CLICK_DURATION};
TButton settingsButton{SETTINGS_SW, LOW, HOLD_DURATION, DEBOUNCE, CLICK_DURATION};
TButton backButton{BACK_SW, LOW, HOLD_DURATION, DEBOUNCE, CLICK_DURATION};
Encoder encoder(ENCODER_PINB, ENCODER_PINA);//This often needs the pins swapping depending on the encoder

void setupHardware()
{
   //Volume Pot is on ADC0
  adc->adc0->setAveraging(16); // set number of averages 0, 4, 8, 16 or 32.
  adc->adc0->setResolution(10); // set bits of resolution  8, 10, 12 or 16 bits.
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_LOW_SPEED); // change the conversion speed
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED); // change the sampling speed

  //MUXs on ADC1
  adc->adc1->setAveraging(16); // set number of averages 0, 4, 8, 16 or 32.
  adc->adc1->setResolution(10); // set bits of resolution  8, 10, 12 or 16 bits.
  adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_LOW_SPEED); // change the conversion speed
  adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED); // change the sampling speed

  //Mux address pins
  pinMode(MUX_0, OUTPUT);
  pinMode(MUX_1, OUTPUT);
  pinMode(MUX_2, OUTPUT);
  pinMode(MUX_3, OUTPUT);

  pinMode(DEMUX_0, OUTPUT);
  pinMode(DEMUX_1, OUTPUT);
  pinMode(DEMUX_2, OUTPUT);
  pinMode(DEMUX_3, OUTPUT);

  digitalWrite(MUX_0, LOW);
  digitalWrite(MUX_1, LOW);
  digitalWrite(MUX_2, LOW);
  digitalWrite(MUX_3, LOW);

  digitalWrite(DEMUX_0, LOW);
  digitalWrite(DEMUX_1, LOW);
  digitalWrite(DEMUX_2, LOW);
  digitalWrite(DEMUX_3, LOW);

  pinMode(DEMUX_EN_1, OUTPUT);
  pinMode(DEMUX_EN_2, OUTPUT);
  pinMode(DEMUX_EN_3, OUTPUT);
  
  digitalWriteFast(DEMUX_EN_1, HIGH);
  digitalWriteFast(DEMUX_EN_2, HIGH);
  digitalWriteFast(DEMUX_EN_3, HIGH);

  analogWriteResolution(10);
  analogReadResolution(10);
  

  //Switches
  pinMode(OCTAVEMOD_SW, INPUT_PULLUP);
  pinMode(SYNC_SW, INPUT_PULLUP);
  pinMode(OCTAVE1_SW, INPUT_PULLUP);
  pinMode(OCTAVE2_SW, INPUT_PULLUP);
  pinMode(RECALL_SW, INPUT_PULLUP); //On encoder
  pinMode(SAVE_SW, INPUT_PULLUP);
  pinMode(SETTINGS_SW, INPUT_PULLUP);
  pinMode(BACK_SW, INPUT_PULLUP);

  //LEDs
  pinMode(SYNC_LED, OUTPUT);
  pinMode(OCTAVE2_LED, OUTPUT);
  pinMode(OCTAVE1_LED, OUTPUT);
  pinMode(OCTAVEMOD_LED, OUTPUT);
}
