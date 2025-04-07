// DRV7781 Motor Driver Control 

#define IN1 D6 //PA14  
#define IN2 D5 //PA4
#define PULSES_PER_REV 784  

volatile long encoderPos = 0;

const int encoderPinA = 2;
const int encoderPinB = 3;

int AMP = 70;
float targetAngle = 0;

void setup() {
  Serial.begin(9600);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

}

void loop() {
  int AMP = 200; //range (50-255) for noticable motor spin
  
  analogWrite(IN1, AMP); 
  analogWrite(IN2, 0);
  delay(5000);  // Run for 2 seconds

  analogWrite(IN1, 0); 
  analogWrite(IN2, AMP);
  delay(5000);
}
