#include <Wire.h>
#include <Adafruit_LSM6DS3TRC.h>

// Create sensor object
Adafruit_LSM6DS3TRC lsm6ds3trc;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // Wait for Serial monitor to open

  // Initialize I2C communication
  if (!lsm6ds3trc.begin_I2C()) {
    Serial.println("Failed to find LSM6DS3TR-C chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("LSM6DS3TR-C Found!");
}

void loop() {
  // Read accelerometer and gyroscope data
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  // Print accelerometer data
  Serial.print("Accel X: ");
  Serial.print(accel.acceleration.x);
  Serial.print(" m/s^2, Y: ");
  Serial.print(accel.acceleration.y);
  Serial.print(" m/s^2, Z: ");
  Serial.print(accel.acceleration.z);
  Serial.println(" m/s^2");

  // Print gyroscope data
  Serial.print("Gyro X: ");
  Serial.print(gyro.gyro.x);
  Serial.print(" rad/s, Y: ");
  Serial.print(gyro.gyro.y);
  Serial.print(" rad/s, Z: ");
  Serial.print(gyro.gyro.z);
  Serial.println(" rad/s");

  Serial.println("------------------------------------");
  delay(500);
}
