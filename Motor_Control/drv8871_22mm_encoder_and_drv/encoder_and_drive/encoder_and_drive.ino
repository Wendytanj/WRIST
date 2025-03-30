#define IN1 6
#define IN2 5 
#define PULSES_PER_REV 784  

volatile long encoderPos = 0;

const int encoderPinA = 2;
const int encoderPinB = 3;

int AMP = 70;
float targetAngle = 0;

void setup() {
  Serial.begin(115200);
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, RISING);
}

void loop() {
  // Handle serial input
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "X") {
      analogWrite(IN1, 0);
      analogWrite(IN2, 0);
    } 
    else if (command == "R") {
      analogWrite(IN1, AMP);
      analogWrite(IN2, 0);
    } 
    else if (command.startsWith("T")) {
      targetAngle = command.substring(1).toFloat();
      Serial.print("Target Angle: ");
      Serial.println(targetAngle);
      rotateAngle(targetAngle, true);
    }
  }

  // Optional debug
  /*
  noInterrupts();
  long pos = encoderPos;
  interrupts();

  float angle = ((float)pos * 360.0) / PULSES_PER_REV;
  while (angle < 0) angle += 360;
  while (angle >= 360) angle -= 360;

  Serial.print("Encoder Count: ");
  Serial.print(pos);
  Serial.print("  Angle: ");
  Serial.print(angle);
  Serial.println("°");

  delay(500);
  */
}

void rotateAngle(float targetAngle, bool clockwise) {
  long startCount;

  int targetPulses = round(targetAngle / (360.0 / PULSES_PER_REV));

  noInterrupts();
  startCount = encoderPos;
  interrupts();

  int minSpeed = 50;   // Minimum speed (must be high enough to move motor)
  int maxSpeed = AMP;  // Your defined max speed

  while (true) {
    noInterrupts();
    long currentCount = encoderPos;
    interrupts();

    long delta = abs(currentCount - startCount);
    long error = targetPulses - delta;

    if (error <= 0) {
      analogWrite(IN1, 255);
      analogWrite(IN2, 255);
      Serial.println("Target reached.");
      break;
    }

    // Proportional speed control: slows down as it nears the target
    int speed = map(error, 0, targetPulses, minSpeed, maxSpeed);
    speed = constrain(speed, minSpeed, maxSpeed);

    if (clockwise) {
      analogWrite(IN1, 0);
      analogWrite(IN2, speed);
    } else {
      analogWrite(IN1, speed);
      analogWrite(IN2, 0);
    }

    Serial.print("Delta: ");
    Serial.print(delta);
    Serial.print("  Error: ");
    Serial.print(error);
    Serial.print("  Speed: ");
    Serial.println(speed);
  }
}


void encoderISR() {
  int bState = digitalRead(encoderPinB);
  if (bState == HIGH) {
    encoderPos++;
  } else {
    encoderPos--;
  }
}
