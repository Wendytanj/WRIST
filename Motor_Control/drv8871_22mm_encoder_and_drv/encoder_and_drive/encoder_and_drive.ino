#define IN1 6
#define IN2 5 
#define PULSES_PER_REV 16  

volatile long encoderPos = 0;    // Global counter for encoder pulses

const int encoderPinA = 2;       // Encoder channel A (interrupt-capable)
const int encoderPinB = 3;       // Encoder channel B

void setup() {
  Serial.begin(9600);

  // Configure encoder pins as inputs with pull-up resistors
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // Set motor driver pins as outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Attach interrupt to encoder channel A using CHANGE edge trigger
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);

  // Start the motor spinning in one direction continuously
  int AMP = 70; // Adjust speed as needed
  analogWrite(IN1, AMP);
  analogWrite(IN2, 0);
}

void loop() {
  // Safely read the encoder counter
  noInterrupts();
  long pos = encoderPos;
  interrupts();

  // Calculate the angle in degrees (cumulative)
  float angle = ((float)pos * 360.0) / PULSES_PER_REV;

  // Normalize angle to 0-360° range
  while (angle < 0) {
    angle += 360;
  }
  while (angle >= 360) {
    angle -= 360;
  }
  
  Serial.print("Encoder Count: ");
  Serial.print(pos);
  Serial.print("  Angle: ");
  Serial.print(angle);
  Serial.println("°");
  
  delay(500);  // Update every 500 ms
}

// Interrupt Service Routine for encoder channel A
void encoderISR() {
  // Read channel B to determine rotation direction
  int bState = digitalRead(encoderPinB);
  
  // Update encoder position based on the direction
  if (bState == HIGH) {
    encoderPos++;  // Clockwise rotation
  } else {
    encoderPos--;  // Counter-clockwise rotation
  }
}
