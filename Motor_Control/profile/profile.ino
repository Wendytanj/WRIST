/*
  TB6612FNG Dual Motor Driver Code with Sine PWM Profile on 'T' Command
*/

// Pin definitions
#define PWM1 5
#define AIN2 6
#define AIN1 7

#define BIN1 8
#define BIN2 9
#define PWM2 10

// Sine wave parameters
const float period = 5.0; // Sine wave period in seconds

// Control state variables
bool sineActive = false;         // Flag indicating if sine modulation is active
unsigned long sineStartTime = 0;   // The time when the sine cycle started

void setup() {
  Serial.begin(9600);
  pinMode(PWM1, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWM2, OUTPUT);
}

void loop() {
  // Check for serial input
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "T") {
      // When 'T' is received, start the sine cycle and set motor direction for tightening.
      sineActive = true;
      sineStartTime = millis();  // Record the starting time for the sine cycle
      
      // Set motor directions for "tighten"
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, HIGH);
      
      Serial.println("Sine cycle started (Tighten).");
      
    } else if (command == "R") {
      // Release motors and disable sine modulation
      sineActive = false;
      
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      digitalWrite(BIN1, HIGH);
      digitalWrite(BIN2, LOW);
      
      // Optionally, set a fixed PWM value when not modulated (or stop the motors)
      analogWrite(PWM1, 0);
      analogWrite(PWM2, 0);
      
      Serial.println("Sine cycle stopped. Motors set to Release.");
      
    } else if (command == "X") {
      // Stop motors (coast) and disable sine modulation
      sineActive = false;
      
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, LOW);
      
      // Stop PWM output
      analogWrite(PWM1, 0);
      analogWrite(PWM2, 0);
      
      Serial.println("Sine cycle stopped. Motors Stopped.");
    }
  }

  // If sine modulation is active, compute and update the PWM duty cycle accordingly.
  if (sineActive) {
    // Calculate the elapsed time since the sine cycle started (in seconds)
    float elapsedTime = (millis() - sineStartTime) / 1000.0;
    
    // Calculate the phase angle (in radians) for the sine function.
    float angle = 2 * PI * elapsedTime / period;
    
    // Compute the sine value (-1.0 to 1.0)
    float sineValue = sin(angle);
    
    // Map sine value from [-1, 1] to PWM range [0, 255]
    int motorSpeed = (int)(((sineValue + 1.0) / 2.0) * 255);
    
    // Write the PWM value to both motors
    analogWrite(PWM1, motorSpeed);
    analogWrite(PWM2, motorSpeed);
    
    // Optionally, print debugging info to the Serial Monitor
    Serial.print("Elapsed: ");
    Serial.print(elapsedTime, 2);
    Serial.print(" s, PWM: ");
    Serial.println(motorSpeed);
  }
  
  // Small delay to stabilize the loop timing
  delay(10);
}
