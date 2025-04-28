#define IN1 6
#define IN2 5
#define PULSES_PER_REV 784

volatile long encoderPos = 0;

// Pins used for the encoder:
const int encoderPinA = 2;  // on the Uno, digital pin 2 corresponds to interrupt 0
const int encoderPinB = 3;

// Default speed
int AMP = 70;
// Default target angle
float targetAngle = 0;

void setup() {
  Serial.begin(115200);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // On the Arduino UNO, attachInterrupt(0, ...) uses digital pin 2.
  attachInterrupt(0, encoderISR, RISING);
}

void loop() {
  // Handle serial input
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "X") {
      // Stop the motor
      analogWrite(IN1, 0);
      analogWrite(IN2, 0);
    } 
    else if (command.startsWith("S")) {
      // Change speed
      int newAmp = command.substring(1).toInt();
      if (newAmp >= 70 && newAmp <= 255) {
        AMP = newAmp;
      }
    }
    else if (command == "R") {
      // Counter-clockwise
      analogWrite(IN1, AMP);
      analogWrite(IN2, 0);
    } 
    else if (command == "L") {
      // Clockwise
      analogWrite(IN1, 0);
      analogWrite(IN2, AMP);
    } 
    else if (command.startsWith("A")) {
      // Rotate clockwise by targetAngle
      targetAngle = command.substring(1).toFloat();
      rotateAngle(targetAngle, true);
    }
    else if (command.startsWith("B")) {
      // Rotate counter-clockwise by targetAngle
      targetAngle = command.substring(1).toFloat();
      rotateAngle(targetAngle, false);
    }
  }

  noInterrupts();
  long pos = encoderPos;
  interrupts();

  float angle = ((float)pos * 360.0) / PULSES_PER_REV;

  // Keep angle in [0..360) range
  while (angle < 0)   angle += 360;
  while (angle >= 360) angle -= 360;

  // If you want to monitor angle and pulse count:
  // Serial.print(encoderPos);
  // Serial.print(",");
  // Serial.println(angle);
}

void rotateAngle(float targetAngle, bool clockwise) {
  long startCount;

  // Calculate how many pulses we need for the requested angle
  int targetPulses = round(targetAngle / (360.0 / PULSES_PER_REV));

  noInterrupts();
  startCount = encoderPos;
  interrupts();

  int minSpeed = 50;   // Minimum speed (must be high enough to move motor)
  int maxSpeed = AMP;  // Your defined maximum speed

  while (true) {
    noInterrupts();
    long currentCount = encoderPos;
    interrupts();

    long delta = abs(currentCount - startCount);
    long error = targetPulses - delta;

    // When we've reached or passed the target number of pulses, stop
    if (error <= 0) {
      // Send "stop" signal
      analogWrite(IN1, 255);
      analogWrite(IN2, 255);
      // Serial.println("Target reached.");
      break;
    }

    // Proportional speed control: slows down as it nears the target
    int speed = map(error, 0, targetPulses, minSpeed, maxSpeed);
    speed = constrain(speed, minSpeed, maxSpeed);

    // Directional control
    if (clockwise) {
      analogWrite(IN1, 0);
      analogWrite(IN2, speed);
    } else {
      analogWrite(IN1, speed);
      analogWrite(IN2, 0);
    }

    // Debug prints if needed:
    // Serial.print("Delta: ");
    // Serial.print(delta);
    // Serial.print("  Error: ");
    // Serial.print(error);
    // Serial.print("  Speed: ");
    // Serial.println(speed);
  }
}

// Interrupt Service Routine for encoder pin A
void encoderISR() {
  // Read pin B to determine direction
  int bState = digitalRead(encoderPinB);
  if (bState == HIGH) {
    encoderPos++;
  } else {
    encoderPos--;
  }
}
