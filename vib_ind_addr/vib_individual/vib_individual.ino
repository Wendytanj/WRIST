#include <Wire.h>
#include "Haptic_Driver.h"

#define MAX_WAVE_SIZE 256

// Create a global Haptic_Driver instance (default address 0x48)
Haptic_Driver hapDrive(0x48);

// Function to (re)initialize the driver at the given I2C address
void setHapticAddress(uint8_t addr) {
  hapDrive = Haptic_Driver(addr);
  if (!hapDrive.begin()) {
    Serial.print("Error: Could not communicate at addr 0x");
    Serial.println(addr, HEX);
    return;
  }
  if (!hapDrive.defaultMotor()) {
    Serial.print("Error: Could not set default motor settings at addr 0x");
    Serial.println(addr, HEX);
    return;
  }
  hapDrive.enableFreqTrack(false);
  hapDrive.setOperationMode(DRO_MODE);
  Serial.print("Haptic driver at 0x");
  Serial.print(addr, HEX);
  Serial.println(" configured.");
}

// Default fixed waveform (for "VIB" command)
const uint8_t default_waveform[] = {
  0x00, // 0%
  0x1E, // 30%
  0x3C, // 60%
  0x5A, // 90%
  0x78, // 120%
  0x96, // 150%
  0xB4, // 180%
  0xD2, // 210%
  0xF0, // 240%
  0xBE, // 190%
  0x9C, // 156%
  0x7A, // 122%
  0x58, // 88%
  0x36, // 54%
  0x14, // 20%
  0x00  // 0%
};
const int default_waveform_length = sizeof(default_waveform)/sizeof(default_waveform[0]);

// Plays the default waveform with 50ms delay per sample.
void vibratePattern() {
  for (int i = 0; i < default_waveform_length; i++) {
    hapDrive.setVibrate(default_waveform[i]);
    delay(50);
  }
  hapDrive.setVibrate(0);
}

// Parses the "WAVE" command.
// Expected format (space-delimited):
//   WAVE <addr> <waveLen> <stepMs> <amp0> <amp1> ... <ampN-1>
//   Example: WAVE 0x48 16 50 0x00 0x1E 0x3C ... 0x00
void parseWaveCommand(String cmd) {
  const int MAX_TOKENS = 300;
  String tokens[MAX_TOKENS];
  int tokenCount = 0;
  
  // Tokenize command by spaces (ignoring extra spaces)
  int startIndex = 0;
  while (startIndex < cmd.length()) {
    int spaceIndex = cmd.indexOf(' ', startIndex);
    if (spaceIndex == -1)
      spaceIndex = cmd.length();
    String token = cmd.substring(startIndex, spaceIndex);
    if (token.length() > 0) {
      tokens[tokenCount++] = token;
    }
    startIndex = spaceIndex + 1;
    if (tokenCount >= MAX_TOKENS) break;
  }
  
  if (tokenCount < 5) {
    Serial.println("Error: WAVE command format incorrect");
    return;
  }
  
  // tokens[0] should be "WAVE"
  // tokens[1] is the address
  String addrStr = tokens[1];
  addrStr.trim();
  if (addrStr.startsWith("0x") || addrStr.startsWith("0X"))
    addrStr = addrStr.substring(2);
  int addr = strtol(addrStr.c_str(), NULL, 16);
  
  // tokens[2] is the number of samples
  int waveLen = tokens[2].toInt();
  if (waveLen > MAX_WAVE_SIZE) {
    Serial.println("Warning: waveLen exceeds buffer size, truncating.");
    waveLen = MAX_WAVE_SIZE;
  }
  
  // tokens[3] is the time step in ms
  int stepMs = tokens[3].toInt();
  
  if (tokenCount < (4 + waveLen)) {
    Serial.println("Error: not enough amplitude data in WAVE command.");
    return;
  }
  
  // Parse amplitude data (assumed to be in hex, e.g., 0x1E)
  uint8_t waveData[MAX_WAVE_SIZE];
  for (int i = 0; i < waveLen; i++) {
    String ampStr = tokens[4 + i];
    ampStr.trim();
    if (ampStr.startsWith("0x") || ampStr.startsWith("0X"))
      ampStr = ampStr.substring(2);
    waveData[i] = (uint8_t)strtol(ampStr.c_str(), NULL, 16);
  }
  
  // Set haptic driver address and play the waveform
  Serial.print("Configuring haptic driver at 0x");
  Serial.println(addr, HEX);
  setHapticAddress((uint8_t)addr);
  
  Serial.print("Playing waveform of length ");
  Serial.print(waveLen);
  Serial.print(" with step ");
  Serial.print(stepMs);
  Serial.println(" ms.");
  
  playWaveform(waveData, waveLen, stepMs);
}

// Plays the waveform stored in waveData, one sample every stepMs ms.
void playWaveform(uint8_t* waveData, int waveLen, int stepMs) {
  for (int i = 0; i < waveLen; i++) {
    hapDrive.setVibrate(waveData[i]);
    delay(stepMs);
  }
  hapDrive.setVibrate(0);
}

void setup() {
  Wire.begin();
  Serial.begin(115200);
  
  // Initialize default haptic driver at address 0x48
  if (!hapDrive.begin()) {
    Serial.println("Could not communicate with default Haptic Driver at 0x48.");
  } else {
    Serial.println("Found DA7281 at 0x48!");
    if (!hapDrive.defaultMotor())
      Serial.println("Could not set default settings on default address.");
    hapDrive.enableFreqTrack(false);
    hapDrive.setOperationMode(DRO_MODE);
    Serial.println("Ready.");
  }
  delay(500);
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() == 0) return;
    
    if (cmd.startsWith("VIB ")) {
      // Legacy command: simply play default waveform on specified address
      String addrStr = cmd.substring(4);
      addrStr.trim();
      if (addrStr.startsWith("0x") || addrStr.startsWith("0X"))
        addrStr = addrStr.substring(2);
      int addr = strtol(addrStr.c_str(), NULL, 16);
      setHapticAddress((uint8_t)addr);
      Serial.print("Using address: 0x");
      Serial.println(addr, HEX);
      vibratePattern();
    } else if (cmd.startsWith("WAVE ")) {
      // New command: parse and play custom waveform
      parseWaveCommand(cmd);
    } else {
      Serial.println("Unknown command.");
    }
  }
}
