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

Haptic_Driver hapDrive;

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

void loop(){
  // Stream the custom waveform
  for(int i = 0; i < waveform_length; i++){
    uint8_t amplitude = custom_waveform[i];
    
    // Write to OVERRIDE_VAL Register
    if(hapDrive.setVibrate(amplitude)){
      Serial.print("OVERRIDE_VAL set to: 0x");
      Serial.println(amplitude, HEX);
    }
    else{
      Serial.print("Failed to set OVERRIDE_VAL at index ");
      Serial.println(i);
    }
    delay(50); // Adjust delay as needed for desired waveform speed
  }
  
  // Optionally loop the waveform indefinitely
}