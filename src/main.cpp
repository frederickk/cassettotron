#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include "MIDIUSB.h"

#define DEBUG 0

#define RX_LED 17
#define PIN_SDA 2
#define PIN_SCL 3

// For Adafruit MCP4725A1 the address is 0x62 (default)
// or 0x63 (ADDR pin tied to VCC)
// For MCP4725A0 the address is 0x60 or 0x61
// For MCP4725A2 the address is 0x64 or 0x65
#define DAC_ADDR 0x62
#define DAC_RESOLUTION (8)
#define VOLT_DELAY 10

#define START 0xFA  // 250
#define CONTINUE 0xFB  // 251
#define STOP 0xFC  // 252
#define NOTE_ON 0x09 // 144
#define NOTE_OFF 0x08 // 128

Adafruit_MCP4725 dac;

double volts_per_note = 0.0833;  // 1/12th V for 1V/Oct

void printMidiMessages(byte channel, byte note, byte velocity) {
#if DEBUG
  Serial.print(channel, DEC);
  Serial.print(" : ");
  Serial.print(note, DEC);
  Serial.print(" : ");
  Serial.print(velocity, DEC);
  Serial.println("");
#endif
}

// Converts MIDI note number to voltage.
uint16_t midi_to_mv(byte note) {
  return 1000 * (note * volts_per_note);
}

// Handles MIDI note on event.x
void noteOnHandler(byte channel, byte note, byte velocity) {
  note -= 60; // set C3 to start of scale

  uint16_t voltage = midi_to_mv(abs(note));

#if DEBUG
  Serial.print("voltage ");
  Serial.print(voltage);
  Serial.print(" : ");
#endif

  printMidiMessages(channel, note, velocity);
  digitalWrite(RX_LED, HIGH);
  dac.setVoltage(voltage, false);
}

// Handles MIDI note off event.
void noteOffHandler(byte channel, byte note, byte velocity) {
  digitalWrite(RX_LED, LOW);
  // printMidiMessages(channel, note, velocity);
  // dac.setVoltage(0, false);
}

// Handles MIDI start/continue/stop events.
void startHandler(byte channel) {
  MidiUSB.flush();
  digitalWrite(PIN_SDA, LOW);
  digitalWrite(PIN_SCL, LOW);
  digitalWrite(RX_LED, LOW);
  dac.setVoltage(0, true);
}

void setup() {
  Serial.begin(31250); // MIDI baudrate.

  pinMode(PIN_SDA, OUTPUT);
  pinMode(PIN_SCL, OUTPUT);
  pinMode(RX_LED, OUTPUT);

  dac.begin(DAC_ADDR);

  startHandler(0);
}

void loop() {
  midiEventPacket_t rx;
  do {
    rx = MidiUSB.read();
    if (rx.header != 0) {
      byte channel = rx.byte1 & 0xF;
      byte note = rx.byte2;
      byte velocity = rx.byte3;

      switch (rx.header & 0x0F) {
        case START:
        case CONTINUE:
        case STOP:
          startHandler(channel);
          break;
        case NOTE_ON:
          noteOnHandler(channel, note, velocity);
          break;
        case NOTE_OFF:
          noteOffHandler(channel, note, velocity);
          break;
      }
    }
  } while (rx.header != 0);

  delay(VOLT_DELAY);
}
