#include <Wire.h>
#include "Haptic_Driver.h"

#define MAX_WAVE_SIZE 256
#define MAX_ADDR 3

// Global driver instance for commands
Haptic_Driver hapDrive(0x48);

// Forward declarations
void parseWCommand(String cmd);
void parseCCommand(String cmd);
void setHapticAddress(uint8_t addr);

//----------------------------------
// setHapticAddress
//----------------------------------
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
}

//----------------------------------
// parseWCommand
//----------------------------------
// Command format:
// W <num_addr> <addr1> <addr2> ... <waveLen> <stepMs> <amp0> ... <ampN-1>
void parseWCommand(String cmd) {
  const int MAX_TOKENS = 300;
  String tokens[MAX_TOKENS];
  int tokenCount = 0;
  
  // Tokenize by spaces
  int startIndex = 0;
  while (startIndex < cmd.length()) {
    int spaceIndex = cmd.indexOf(' ', startIndex);
    if (spaceIndex == -1)
      spaceIndex = cmd.length();
    String token = cmd.substring(startIndex, spaceIndex);
    token.trim();
    if (token.length() > 0) {
      tokens[tokenCount++] = token;
    }
    startIndex = spaceIndex + 1;
    if (tokenCount >= MAX_TOKENS) break;
  }
  
  if (tokenCount < 6) {
    Serial.println("Error: Command format incorrect");
    return;
  }
  
  int numAddr = tokens[1].toInt();
  if (numAddr < 1 || numAddr > MAX_ADDR) {
    Serial.println("Error: Invalid number of addresses");
    return;
  }
  
  // Read addresses
  uint8_t addrs[MAX_ADDR];
  for (int i = 0; i < numAddr; i++) {
    String addrStr = tokens[2 + i];
    if (addrStr.startsWith("0x") || addrStr.startsWith("0X"))
      addrStr = addrStr.substring(2);
    addrs[i] = (uint8_t)strtol(addrStr.c_str(), NULL, 16);
  }
  
  int waveLenIndex = 2 + numAddr;
  int waveLen = tokens[waveLenIndex].toInt();
  if (waveLen > MAX_WAVE_SIZE) {
    Serial.println("Warning: waveLen exceeds buffer size, truncating.");
    waveLen = MAX_WAVE_SIZE;
  }
  
  int stepMsIndex = 3 + numAddr;
  int stepMs = tokens[stepMsIndex].toInt();
  
  // The remaining tokens are amplitude values
  int firstAmpIndex = 4 + numAddr;
  if (tokenCount < firstAmpIndex + waveLen) {
    Serial.println("Error: Not enough amplitude data in command.");
    return;
  }
  
  uint8_t waveData[MAX_WAVE_SIZE];
  for (int i = 0; i < waveLen; i++) {
    String ampStr = tokens[firstAmpIndex + i];
    waveData[i] = (uint8_t)strtol(ampStr.c_str(), NULL, 16);
  }
  
  // We'll record the last time we called setVibrate() for each address
  unsigned long deviceTime[MAX_ADDR] = {0, 0, 0};
  
  // Start time for entire command
  unsigned long startTime = millis();
  
  // For each sample
  for (int i = 0; i < waveLen; i++) {
    // For each address
    for (int j = 0; j < numAddr; j++) {
      setHapticAddress(addrs[j]);
      hapDrive.setVibrate(waveData[i]);
      deviceTime[j] = millis();  // record time right after setVibrate
    }
    delay(stepMs);
  }
  
  // Turn off all devices
  for (int j = 0; j < numAddr; j++) {
    setHapticAddress(addrs[j]);
    hapDrive.setVibrate(0);
  }
  
  unsigned long endTime = millis();
  unsigned long diff = endTime - startTime;
  
  // Print "DONE <time0> <time1> <time2> <diff>"
  // If numAddr < 3, some times might remain 0.
  Serial.print("DONE ");
  for (int j = 0; j < numAddr; j++) {
    Serial.print(deviceTime[j]);
    Serial.print(" ");
  }
  Serial.println(diff);
}

//----------------------------------
// parseCCommand
//----------------------------------
// Format: C <num_addr> <addr1> <addr2> ... <amp>
void parseCCommand(String cmd) {
  const int MAX_TOKENS = 10;
  String tokens[MAX_TOKENS];
  int tokenCount = 0;
  int startIndex = 0;
  
  while (startIndex < cmd.length()) {
    int spaceIndex = cmd.indexOf(' ', startIndex);
    if (spaceIndex == -1)
      spaceIndex = cmd.length();
    String token = cmd.substring(startIndex, spaceIndex);
    token.trim();
    if (token.length() > 0) {
      tokens[tokenCount++] = token;
    }
    startIndex = spaceIndex + 1;
    if (tokenCount >= MAX_TOKENS) break;
  }
  
  if (tokenCount < 3) {
    Serial.println("Error: C command format incorrect.");
    return;
  }
  
  int numAddr = tokens[1].toInt();
  if (numAddr < 1 || numAddr > MAX_ADDR) {
    Serial.println("Error: Invalid number of addresses.");
    return;
  }
  
  if (tokenCount < 2 + numAddr + 1) {
    Serial.println("Error: Not enough tokens for C command.");
    return;
  }
  
  int amplitude = tokens[2 + numAddr].toInt();
  
  unsigned long startTime = millis();
  
  // Optionally record times for each address if you want
  unsigned long deviceTime[MAX_ADDR] = {0, 0, 0};
  
  for (int i = 0; i < numAddr; i++) {
    String addrStr = tokens[2 + i];
    if (addrStr.startsWith("0x") || addrStr.startsWith("0X"))
      addrStr = addrStr.substring(2);
    int addr = strtol(addrStr.c_str(), NULL, 16);
    setHapticAddress((uint8_t)addr);
    hapDrive.setVibrate((uint8_t)amplitude);
    deviceTime[i] = millis();
  }
  
  unsigned long endTime = millis();
  unsigned long diff = endTime - startTime;
  
  // Print "DONE <time0> <time1> <time2> <diff>"
  Serial.print("DONE ");
  for (int i = 0; i < numAddr; i++) {
    Serial.print(deviceTime[i]-startTime);
    Serial.print(" ");
  }
  Serial.println(diff);
}

//----------------------------------
// setup, loop
//----------------------------------
void setup() {
  Wire.begin();
  Serial.begin(115200);
  
  if (!hapDrive.begin()) {
    Serial.println("Could not communicate with default Haptic Driver at 0x48.");
  } else {
    Serial.println("Found DA7281 at 0x48!");
    if (!hapDrive.defaultMotor()) {
      Serial.println("Could not set default settings on default address.");
    }
    hapDrive.enableFreqTrack(false);
    hapDrive.setOperationMode(DRO_MODE);
    Serial.println("Ready. Waiting for commands...");
  }
  delay(500);
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() == 0) return;
    
    if (cmd.startsWith("W ")) {
      parseWCommand(cmd);
    } else if (cmd.startsWith("C ")) {
      parseCCommand(cmd);
    } else {
      Serial.println("Unknown command.");
    }
  }
}
