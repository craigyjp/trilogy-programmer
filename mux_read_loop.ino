#include <Arduino.h>
// #include <U8g2lib.h>
#include <TM1637Display.h>
// #include <SPI.h>
#include <ResponsiveAnalogRead.h>
#include <Bounce.h>
#include <Wire.h>


// U8G2_SSD1327_EA_W128128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 35, /* data=*/ 36, /* cs=*/ 37, /* dc=*/ 38, /* reset=*/ 23);


// Module connection pins (Digital Pins)
#define CLK 29
#define DIO 30

const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};

TM1637Display display(CLK, DIO);


// Multiplexer selector pins on teensy
int SELECTOR_PINS[] = {2, 3, 4, 5};

// pins to read
const int AN = 2; // number of analog pins
const int DN = 3; // number of digital pins
const int AO = 2; // number of analog pins out
const int DO = 2; // number of digital pins out
const int ANALOG_PINS[AN] = {A0, A1}; // teensy mapping
const int DIGITAL_PINS[DN] = {6, 7, 8}; // teensy mapping
const int ANALOG_PINS_OUT[AO] = {A2, A3}; // teensy mapping
const int DIGITAL_PINS_OUT[DO] = {9, 10}; // teensy mapping

const int MAX_PINS = 16; // max number of pins multiplexed per multiplexer
const int ANALOG_MUX_PINS[AN] = {16, 16}; // number of pins on multiplexer used for signals
const int DIGITAL_MUX_PINS[DN] = {16, 16, 16}; // number of pins on multiplexer used for signals
const int ANALOG_MUX_PINS_OUT[AO] = {16, 16}; // number of pins on multiplexer used for signals
const int DIGITAL_MUX_PINS_OUT[DO] = {16, 16}; // number of pins on multiplexer used for signals

// analog[i][j] corresponds to analog pin i on Teensy, pin j on multiplexer
ResponsiveAnalogRead *analog[AN][MAX_PINS];
byte analog_data[AN][MAX_PINS];
byte analog_data_prev[AN][MAX_PINS];

// digital[i][j] corresponds to digital pin i on Teensy, pin j on multiplexer
Bounce *digital[DN][MAX_PINS];

const int BOUNCE_TIME = 5; // 5 ms is usually sufficient
const int ON_VELOCITY = 127; // pressed button "note" velocity

void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT); // address 0  > D0 > pin2
  pinMode(3, OUTPUT); // address 1  > D1 > pin3
  pinMode(4, OUTPUT); // address 2  > D2 > pin4
  pinMode(5, OUTPUT); // address 3  > D3 > pin5
  Wire.setSDA(34);
  Wire.setSCL(33);
//  setupDisplay();
  setupAnalog();
  setupDigital();
}

// void setupDisplay() {
//  u8g2.begin();
//  u8g2.clearBuffer();          // clear the internal memory
//  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
//  u8g2.drawStr(0, 20, "Crumar Trilogy!"); // write something to the internal memory
//  u8g2.sendBuffer();          // transfer internal memory to the display
// }

void setupAnalog() {
  for (int i = 0; i < AN; i++) {
    for (int j = 0; j < ANALOG_MUX_PINS[i]; j++) {
      analog[i][j] = new ResponsiveAnalogRead(ANALOG_PINS[i], true); // initialize
    }
  }
}

void setupDigital() {
  for (int i = 0; i < DN; i++) {
    for (int j = 0; j < DIGITAL_MUX_PINS[i]; j++) {
      digital[i][j] = new Bounce(DIGITAL_PINS[i], true); // initialize
      // digital[i][j] = new Bounce(DIGITAL_PINS_OUT[i], true); // initialize
    }
    pinMode(DIGITAL_PINS[i], INPUT_PULLUP); // internal pullup resistors for digital section
  }
}

void loop() {

  //  uint8_t data[] = { 0x01, 0x02, 0xff, 0xff };
  //  uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };
  display.setBrightness(0x0f);
  //display.showNumberDec(0, true);

  for (int j = 0; j < MAX_PINS; j++) { // loop over multiplexer
    setSelectorPins(j);
    // display.showNumberDec(j, true);
    readAnalogs(j);
    readDigitals(j);
    delayMicroseconds(1000);
  }
}

void readAnalogs(int j) {
  for (int i = 0; i < AN; i++) { // loop over analog inputs
    if (j < ANALOG_MUX_PINS[i]) { // pin j on multiplexer is actually used
      analog[i][j]->update();
      if (analog[i][j]->hasChanged()) {
        analog_data[i][j] = analog[i][j]->getValue() >> 3;
        if (analog_data[i][j] != analog_data_prev[i][j]) {
          analog_data_prev[i][j] = analog_data[i][j];
          // Serial.print("j   ");
          // Serial.println(j);
          // Serial.print("Analogue Data  ");
          // Serial.println(analog_data[i][j]);
          //          usbMIDI.sendControlChange(16 + j, analog_data[i][j], i + 1);
        }
      }
    }
  }
}

void readDigitals(int j) {
  for (int i = 0; i < DN; i++) { // loop over digital inputs
    if (j < DIGITAL_MUX_PINS[i]) { // pin j on multiplexer is actually used
      digital[i][j]->update();
      if (digital[i][j]->fallingEdge()) {
        // usbMIDI.sendNoteOn(j+i*MAX_PINS, ON_VELOCITY, AN + i + 1);
        // display.showNumberDec(j + i * MAX_PINS, true);
          Serial.print("j   ");
          Serial.println(j);
          Serial.print("Digital Inputs  ");
          Serial.println(j + i * MAX_PINS);
        // note is 16*current digital teensy pin index (i) + current mux pin index (j)
        // velocity is pressing a note (should be 65 to 127)
        // channel is one above the analog + index of current digital teensy pin index (i)
      }

      if (digital[i][j]->risingEdge()) {
        // usbMIDI.sendNoteOff(i+j*MAX_PINS, 0, AN + i + 1);
        //        display.showNumberDec(0, true);
      }
    }
  }
}

void setSelectorPins(int j) {
  byte s0 = (j & B0001) ? HIGH : LOW;
  byte s1 = (j & B0010) ? HIGH : LOW;
  byte s2 = (j & B0100) ? HIGH : LOW;
  byte s3 = (j & B1000) ? HIGH : LOW;
  digitalWrite(SELECTOR_PINS[0], s0);
  digitalWrite(SELECTOR_PINS[1], s1);
  digitalWrite(SELECTOR_PINS[2], s2);
  digitalWrite(SELECTOR_PINS[3], s3);

  // allow 50 us for signals to stablize
  delayMicroseconds(1000);
}
