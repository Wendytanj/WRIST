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
#define FORCE_ADDR       0x04 
#define IN1 PA1  
#define IN2 PA5  

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
  // I2C
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

  // PWM
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
}

void loop(){
  //Motor Spin Forward
  int AMP = 50; //range (50-255) for noticable motor spin
  
  analogWrite(IN1, AMP); 
  analogWrite(IN2, 0);

  //analogWrite(IN1, 0); 
 // analogWrite(IN2, AMP);
  //delay(5000);

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

  delay(5);

  // Force Sensor I2C
  // Request 6 bytes from the SingleTact sensor
  // which correspond to registers [128..133].
  Wire.requestFrom(FORCE_ADDR, (uint8_t)6);

  // We expect exactly 6 bytes. If the sensor or wiring
  // is not correct, we may get fewer bytes.
  byte data[6] = {0};
  int index = 0;
  while (Wire.available() && index < 6) {
    data[index++] = Wire.read();
  }

  if (index == 6) {
    // Parse the 6 bytes
    // data[0..1] = Frame index
    uint16_t frameIndex = (uint16_t(data[0]) << 8) | data[1];

    // data[2..3] = Timestamp (0.1 ms increments, approximate)
    uint16_t timeStamp = (uint16_t(data[2]) << 8) | data[3];

    // data[4..5] = 10-bit sensor output (0..1023)
    uint16_t rawOutput = (uint16_t(data[4]) << 8) | data[5];

    Serial.print("FrameIdx: ");
    Serial.print(frameIndex);
    Serial.print(" | TimeStamp: ");
    Serial.print(timeStamp);
    Serial.print(" | SensorOut: ");
    Serial.println(rawOutput);
  } else {
    Serial.println("Not enough data received from SingleTact!");
  }

  delay(200);  // Adjust as needed (5-10 Hz polling, etc.)
  
  // Optionally loop the waveform indefinitely
}