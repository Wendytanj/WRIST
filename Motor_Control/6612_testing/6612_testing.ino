/*
  TB6612FNG Dual Motor Driver Code
*/

// Pin definitions
#define PWM1 5
#define AIN2 6
#define AIN1 7

#define BIN1 8
#define BIN2 9
#define PWM2 10

int pot;
int out;

void setup() {
  Serial.begin(9600);
  pinMode(PWM1,OUTPUT);
  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT);
  pinMode(BIN1,OUTPUT);
  pinMode(BIN2,OUTPUT);
  pinMode(PWM2, OUTPUT);
}
 
void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command == "T") {
      // Tighten motors
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, HIGH);
    } else if (command == "R") {
      // Release motors
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      digitalWrite(BIN1, HIGH);
      digitalWrite(BIN2, LOW);
    } else if (command == "X") {
      // Stop motors
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, LOW);
    }
  }

  int motorSpeed = 128; 
  analogWrite(PWM1, motorSpeed);
  analogWrite(PWM2, motorSpeed);
}
