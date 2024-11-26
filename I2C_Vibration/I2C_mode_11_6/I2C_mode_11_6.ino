/*
Date: 5/2021
Author: Elias Santistevan @ SparkFun Electronics
Writing vibration values via I2C to vibrate the motor. 

This vibrates *extremely* vigurously, adhere the motor to something or it will
produce a fault and stop functioning. 

*/

#include <Wire.h>
#include "Haptic_Driver.h"

Haptic_Driver hapDrive;

int event = 0; 

const uint8_t custom_waveform[] = {
  0x00, // 0%
  0x1E, // 30%
  0x3C, // 60%
  0x5A, // 90%
  0x78, // 120% (Capped at 100% if necessary)
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

  if( !hapDrive.begin())
    Serial.println("Could not communicate with Haptic Driver.");
  else
    Serial.println("Found DA7280!");

  if( !hapDrive.defaultMotor() ) 
    Serial.println("Could not set default settings.");

  // Disable resonant freq tracking
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

    // Delay between updates (adjust as needed for desired waveform speed)
    delay(50); // 50 ms
  }
  
  // Optionally, loop the waveform indefinitely
  // If you want to stop after one iteration, remove the while loop
}
