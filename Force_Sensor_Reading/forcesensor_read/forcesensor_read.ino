#include <Wire.h>
#define SINGLETACT_ADDR 0x04

// We request a 6-byte packet (as per the working .NET demo)
const byte i2cPacketLength = 6;

// Conversion constant: full-scale force in newtons.
// Adjust this value to match your sensor's calibration.
// For example, if 511 corresponds to 10 N full-scale, then NB_TO_FORCE_FACTOR = 10.0.
const float NB_TO_FORCE_FACTOR = 100.0;

void setup() {
  Wire.begin();                 // Initialize I²C as master
  Serial.begin(57600);          // Use same baud rate as original demo
  while (!Serial) {
    ; // wait for Serial port to connect (needed for some boards)
  }
  
  Serial.println("PPS UK: SingleTact sensor");
  Serial.println("Raw value, Force (PSI), and Force (N)");
  Serial.println("----------------------------------------");
}

void loop() {
  short rawData = readDataFromSensor(SINGLETACT_ADDR);
  
  if (rawData == -1) {
    Serial.println("Error: No response from sensor.");
  } else {
    // Subtract offset (0xFF = 255) as in the .NET frame conversion
    short adjustedRaw = rawData - 255;
    
    // Option 1: Display as "PSI" (the demo text used PSI even though later conversion is to newtons)
    // In our example, the raw value in "NB" goes from 0 (0 PSI) to 511 (full-scale).
    // Here we simply output the adjusted raw value.
    short forcePSI = adjustedRaw;  
    
    // Option 2: Convert to force (N) using the conversion factor.
    // This matches the GUI's conversion: Force (N) = (raw - 255) * (NB_TO_FORCE_FACTOR / 512)
    float forceN = adjustedRaw * NB_TO_FORCE_FACTOR / 512.0;
    
    Serial.print("Raw reading: ");
    Serial.print(rawData);
    Serial.print("   Force (PSI - NB): ");
    Serial.print(forcePSI);
    Serial.print("   Force (N): ");
    Serial.println(forceN);
  }
  
  delay(100);  // Delay between readings; adjust as needed
}

/// <summary>
/// Sends a command packet to the sensor and reads 6 bytes in response.
/// The command packet is 3 bytes:
///   Byte 0: Read command (0x01)
///   Byte 1: Data offset (128)
///   Byte 2: Number of bytes to read (6)
///
/// The sensor returns 6 bytes, and the raw force data is contained in
/// bytes 4 and 5 (indices 4 and 5).
/// </summary>
short readDataFromSensor(byte address) {
  byte outgoingI2CBuffer[3];
  outgoingI2CBuffer[0] = 0x01;        // I2C read command
  outgoingI2CBuffer[1] = 128;         // Data offset (as used by the calibration board)
  outgoingI2CBuffer[2] = i2cPacketLength;  // Request 6 bytes

  Wire.beginTransmission(address);
  Wire.write(outgoingI2CBuffer, 3);
  byte error = Wire.endTransmission();  // End transmission and check for errors
  if (error != 0) {
    return -1; // Return -1 on error (e.g., sensor not responding)
  }
  
  // Request 6 bytes from the sensor
  Wire.requestFrom(address, i2cPacketLength);
  byte incomingI2CBuffer[6];
  byte incomeCount = 0;
  while (incomeCount < i2cPacketLength) {
    if (Wire.available()) {
      incomingI2CBuffer[incomeCount] = Wire.read();
      incomeCount++;
    } else {
      delayMicroseconds(10); // Wait a short while if no data available yet
    }
  }
  
  // Combine the 5th and 6th bytes into a 16-bit raw value.
  short rawData = (incomingI2CBuffer[4] << 8) | incomingI2CBuffer[5];
  
  return rawData;
}
