// DRV7781 Motor Driver Control 

#define IN1 PA14 //D6  
#define IN2 PA4 //D5

void setup() {
  Serial.begin(9600);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

}

void loop() {
  int AMP = 70; //range (50-255) for noticable motor spin
  
  analogWrite(IN1, AMP); 
  analogWrite(IN2, 0);
  delay(5000);  // Run for 2 seconds

  analogWrite(IN1, 0); 
  analogWrite(IN2, AMP);
  delay(5000);
}
