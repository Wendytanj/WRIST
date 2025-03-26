#include <Wire.h>
#include "Haptic_Driver.h"

#define MAX_WAVE_SIZE 256
#define NUM_DRIVERS 3

// Pre-created global instances for each address
Haptic_Driver hapDrive48(0x48);
Haptic_Driver hapDrive49(0x49);
Haptic_Driver hapDrive4A(0x4A);

// Helper: returns a pointer to the appropriate driver instance based on address.
Haptic_Driver* getDriver(uint8_t addr) {
  if (addr == 0x48) return &hapDrive48;
  else if (addr == 0x49) return &hapDrive49;
  else if (addr == 0x4A) return &hapDrive4A;
  else return NULL;
}

// Initializes a single driver instance (for all three pre-created objects).
void initDrivers() {
  // Initialize driver at 0x48
  if (!hapDrive48.begin()) {
    Serial.println("Error: Could not communicate with driver 0x48");
  } else {
    if (!hapDrive48.defaultMotor())
      Serial.println("Error: Could not set default settings on 0x48");
    hapDrive48.enableFreqTrack(false);
    hapDrive48.setOperationMode(DRO_MODE);
  }
  
  // Initialize driver at 0x49
  if (!hapDrive49.begin()) {
    Serial.println("Error: Could not communicate with driver 0x49");
  } else {
    if (!hapDrive49.defaultMotor())
      Serial.println("Error: Could not set default settings on 0x49");
    hapDrive49.enableFreqTrack(false);
    hapDrive49.setOperationMode(DRO_MODE);
  }
  
  // Initialize driver at 0x4A
  if (!hapDrive4A.begin()) {
    Serial.println("Error: Could not communicate with driver 0x4A");
  } else {
    if (!hapDrive4A.defaultMotor())
      Serial.println("Error: Could not set default settings on 0x4A");
    hapDrive4A.enableFreqTrack(false);
    hapDrive4A.setOperationMode(DRO_MODE);
  }
}

//-------------------------------
// parseWCommand
//-------------------------------
// New waveform command format: 
// W <num_addr> <addr1> <addr2> ... <waveLen> <stepMs> <amp0> ... <ampN-1>
void parseWCommand(String cmd) {
  const int MAX_TOKENS = 300;
  String tokens[MAX_TOKENS];
  int tokenCount = 0;
  
  // Tokenize the command by spaces.
  int startIndex = 0;
  while (startIndex < cmd.length()) {
    int spaceIndex = cmd.indexOf(' ', startIndex);
    if (spaceIndex == -1)
      spaceIndex = cmd.length();
    String token = cmd.substring(startIndex, spaceIndex);
    token.trim();
    if (token.length() > 0)
      tokens[tokenCount++] = token;
    startIndex = spaceIndex + 1;
    if (tokenCount >= MAX_TOKENS) break;
  }
  
  if (tokenCount < 6) {
    Serial.println("Error: W command format incorrect");
    return;
  }
  
  int numAddr = tokens[1].toInt();
  if (numAddr < 1 || numAddr > NUM_DRIVERS) {
    Serial.println("Error: Invalid number of addresses");
    return;
  }
  
  // Read addresses.
  uint8_t addrs[NUM_DRIVERS];
  for (int i = 0; i < numAddr; i++) {
    String addrStr = tokens[2 + i];
    addrStr.trim();
    if (addrStr.startsWith("0x") || addrStr.startsWith("0X"))
      addrStr = addrStr.substring(2);
    addrs[i] = (uint8_t)strtol(addrStr.c_str(), NULL, 16);
  }
  
  int waveLen = tokens[2 + numAddr].toInt();
  if (waveLen > MAX_WAVE_SIZE) {
    Serial.println("Warning: waveLen exceeds buffer size, truncating.");
    waveLen = MAX_WAVE_SIZE;
  }
  
  int stepMs = tokens[3 + numAddr].toInt();
  
  int firstAmpIndex = 4 + numAddr;
  if (tokenCount < firstAmpIndex + waveLen) {
    Serial.println("Error: Not enough amplitude data in command.");
    return;
  }
  
  uint8_t waveData[MAX_WAVE_SIZE];
  for (int i = 0; i < waveLen; i++) {
    String ampStr = tokens[firstAmpIndex + i];
    ampStr.trim();
    waveData[i] = (uint8_t)strtol(ampStr.c_str(), NULL, 16);
  }
  
  unsigned long startTime = millis();
  // For each sample, update all selected drivers.
  for (int i = 0; i < waveLen; i++) {
    for (int j = 0; j < numAddr; j++) {
      Haptic_Driver* drv = getDriver(addrs[j]);
      if (drv != NULL) {
        drv->setVibrate(waveData[i]);
      }
    }
    delay(stepMs);
  }
  
  // Turn off all drivers.
  for (int j = 0; j < numAddr; j++) {
    Haptic_Driver* drv = getDriver(addrs[j]);
    if (drv != NULL) {
      drv->setVibrate(0);
    }
  }
  
  unsigned long diff = millis() - startTime;
  Serial.print("DONE ");
  Serial.println(diff);
}

//-------------------------------
// parseCCommand
//-------------------------------
// New constant buzz command format: 
// C <num_addr> <addr1> <addr2> ... <amp>
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
    if (token.length() > 0)
      tokens[tokenCount++] = token;
    startIndex = spaceIndex + 1;
    if (tokenCount >= MAX_TOKENS) break;
  }
  
  if (tokenCount < 3) {
    Serial.println("Error: C command format incorrect.");
    return;
  }
  
  int numAddr = tokens[1].toInt();
  if (numAddr < 1 || numAddr > NUM_DRIVERS) {
    Serial.println("Error: Invalid number of addresses.");
    return;
  }
  
  if (tokenCount < 2 + numAddr + 1) {
    Serial.println("Error: Not enough tokens for C command.");
    return;
  }
  
  int amplitude = tokens[2 + numAddr].toInt();
  
  unsigned long startTime = millis();
  for (int i = 0; i < numAddr; i++) {
    String addrStr = tokens[2 + i];
    addrStr.trim();
    if (addrStr.startsWith("0x") || addrStr.startsWith("0X"))
      addrStr = addrStr.substring(2);
    int addr = strtol(addrStr.c_str(), NULL, 16);
    Haptic_Driver* drv = getDriver(addr);
    if (drv != NULL) {
      drv->setVibrate((uint8_t)amplitude);
    }
  }
  unsigned long diff = millis() - startTime;
  Serial.print("DONE ");
  Serial.println(diff);
}

void setup() {
  Wire.begin();
  Serial.begin(115200);
  
  // Pre-initialize all drivers.
  initDrivers();
  
  Serial.println("Ready. Waiting for commands...");
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
