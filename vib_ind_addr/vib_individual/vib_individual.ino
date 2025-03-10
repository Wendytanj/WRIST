/*
  Date: 5/2021 (Adapted for DA7281)
  Author: Elias Santistevan @ SparkFun Electronics
          Adapted by [Your Name]
  
  This sketch writes vibration values via I2C to vibrate the motor.
  NOTE: The original library was for the DA7280.
  For the DA7281, update the library constants as follows:
    - Set CHIP_REV to 0xCA instead of 0xBA.
    - Ensure the I²C address remains 0x48.
*/

#include <Wire.h>
#include "Haptic_Driver.h"

Haptic_Driver hapDrive(0x48);  // default

void setHapticAddress(uint8_t addr) {
  hapDrive = Haptic_Driver(addr);
  hapDrive.begin();
  hapDrive.defaultMotor();
  hapDrive.enableFreqTrack(false);
  hapDrive.setOperationMode(DRO_MODE);
}

const uint8_t custom_waveform[] = {
  0x00, // 0%
  0x1E, // 30%
  0x3C, // 60%
  0x5A, // 90%
  0x78, // 120% (capped at 100% if necessary)
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
const int waveform_length = sizeof(custom_waveform) / sizeof(custom_waveform[0]);

void setup(){
  Wire.begin();
  Serial.begin(115200);

  if( !hapDrive.begin() )
    Serial.println("Could not communicate with Haptic Driver.");
  else
    Serial.println("Found DA7281!");

  if( !hapDrive.defaultMotor() ) 
    Serial.println("Could not set default settings.");

  // Disable resonant frequency tracking (if supported by your hardware)
  hapDrive.enableFreqTrack(false);

  Serial.println("Setting I2C Operation.");
  hapDrive.setOperationMode(DRO_MODE);
  Serial.println("Ready.");
  delay(1000);
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    if (cmd.startsWith("VIB ")) {
      String addrStr = cmd.substring(4);
      addrStr.trim(); // Remove any extra whitespace
      if (addrStr.startsWith("0x") || addrStr.startsWith("0X")) {
        addrStr = addrStr.substring(2);  // Remove "0x"
      }
      int addr = strtol(addrStr.c_str(), NULL, 16); // Convert hex string to int
      setHapticAddress((uint8_t)addr);
      Serial.println(addr); // This should now print 72 for 0x48
      vibratePattern();
    }
  }
}


void vibratePattern() {
  for(int i = 0; i < waveform_length; i++){
    uint8_t amplitude = custom_waveform[i];
    hapDrive.setVibrate(amplitude);
    delay(50);
  }
}
