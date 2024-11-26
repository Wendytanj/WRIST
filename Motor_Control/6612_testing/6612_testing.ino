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

#define BUTTON1_PIN 12
#define BUTTON2_PIN 13

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
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
}
 
void loop() {
  int button1State = digitalRead(BUTTON1_PIN);
  int button2State = digitalRead(BUTTON2_PIN);

  if (button1State == LOW) {
    // Tighten
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  } else if (button2State == LOW) {
    // Release
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else {
    // Stop motors
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
  }
  
  int motorSpeed = 255; 
  analogWrite(PWM1, motorSpeed);
  analogWrite(PWM2, motorSpeed);
}
