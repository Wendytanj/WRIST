// -------------------- PIN DEFINITIONS --------------------
// Motor driver pins
#define IN1 6        
#define IN2 5     

// Encoder pins (STM32)
#define encoderPinA A4 // PB11 on your board
#define encoderPinB 10 // PB0 on your board

// --------------------- CONSTANTS -------------------------
#define PULSES_PER_REV 784  

// --------------------- GLOBALS ---------------------------
volatile long encoderPos = 0; // Accumulates encoder counts
int AMP = 70;                 // Motor PWM value (70..255)
float targetAngle = 0;

// --------------------- SETUP -----------------------------
void setup() {
  Serial.begin(115200);

  // Set up encoder pins as inputs with pull-ups
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // Motor outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Attach interrupt on rising edge of A
  // We'll read B to decide direction
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, RISING);

  // If your board/core doesn’t allow digitalPinToInterrupt(A4),
  // see “Troubleshooting” below.
}

// ---------------------- MAIN LOOP ------------------------
void loop() {

  // ---------------- Handle serial commands ----------------
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "X") {
      // Stop motor
      analogWrite(IN1, 0);
      analogWrite(IN2, 0);
    } 
    else if (command.startsWith("S")) {
      // Change motor speed
      int newAmp = command.substring(1).toInt();
      if (newAmp >= 70 && newAmp <= 255) {
        AMP = newAmp;
      }
    }
    else if (command == "R") {
      // Rotate counter-clockwise
      analogWrite(IN1, AMP);
      analogWrite(IN2, 0);
    } 
    else if (command == "L") {
      // Rotate clockwise
      analogWrite(IN1, 0);
      analogWrite(IN2, AMP);
    } 
    else if (command.startsWith("A")) {
      // Move "clockwise" a given angle
      targetAngle = command.substring(1).toFloat();
      rotateAngle(targetAngle, true);
    }
    else if (command.startsWith("B")) {
      // Move "counter-clockwise" a given angle
      targetAngle = command.substring(1).toFloat();
      rotateAngle(targetAngle, false);
    }
  }

  // ------------------- Read encoder -------------------
  // (If you need to do anything else with the encoder value)
  noInterrupts();
  long pos = encoderPos;
  interrupts();

  float angle = ((float)pos * 360.0) / PULSES_PER_REV;
  while (angle < 0)   angle += 360;
  while (angle >= 360) angle -= 360;

  // (Print debugging if desired)
  // Serial.print(encoderPos);
  // Serial.print(",");
  // Serial.println(angle);

  // Rest of your loop logic...
}

// ------------------ ROTATE FUNCTION ----------------------
void rotateAngle(float targetAngle, bool clockwise) {
  long startCount;

  int targetPulses = round(targetAngle / (360.0 / PULSES_PER_REV));

  noInterrupts();
  startCount = encoderPos;
  interrupts();

  int minSpeed = 50;   // Minimum speed (to overcome motor friction)
  int maxSpeed = AMP;  // Your defined max speed

  while (true) {
    noInterrupts();
    long currentCount = encoderPos;
    interrupts();

    long delta = abs(currentCount - startCount);
    long error = targetPulses - delta;

    if (error <= 0) {
      // Stop motor
      analogWrite(IN1, 255);
      analogWrite(IN2, 255);
      break;
    }

    // Slow down as we near target
    int speed = map(error, 0, targetPulses, minSpeed, maxSpeed);
    speed = constrain(speed, minSpeed, maxSpeed);

    if (clockwise) {
      analogWrite(IN1, 0);
      analogWrite(IN2, speed);
    } else {
      analogWrite(IN1, speed);
      analogWrite(IN2, 0);
    }
  }
}

// ----------------- ENCODER ISR ----------------------
void encoderISR() {
  // On each rising edge of A, read B to decide direction
  int bState = digitalRead(encoderPinB);
  if (bState == HIGH) {
    encoderPos++;
  } else {
    encoderPos--;
  }
}
