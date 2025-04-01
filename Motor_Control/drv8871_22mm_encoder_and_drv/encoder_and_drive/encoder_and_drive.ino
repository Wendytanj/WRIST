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
    else if (command.startsWith("S")) {
    int newAmp = command.substring(1).toInt();
    if (newAmp >= 70 && newAmp <= 255) {
    AMP = newAmp;
    //Serial.print("Speed set to: ");
    //Serial.println(AMP);
    } 
    //else {
    //Serial.println("Invalid speed. Must be between 70 and 255.");
    //}
    }
    else if (command == "R") {
      // counter clockwise
      analogWrite(IN1, AMP);
      analogWrite(IN2, 0);
    } 
    else if (command == "L") {
      //clockwise
      analogWrite(IN1, 0);
      analogWrite(IN2, AMP);
    } 
    else if (command.startsWith("A")) {
      targetAngle = command.substring(1).toFloat();
      //Serial.print("Target Angle: ");
      //Serial.println(targetAngle);
      rotateAngle(targetAngle, true);
    }
    else if (command.startsWith("B")) {
      targetAngle = command.substring(1).toFloat();
      //Serial.print("Target Angle: ");
      //Serial.println(targetAngle);
      rotateAngle(targetAngle, false);
    }
  }

  noInterrupts();
  long pos = encoderPos;
  interrupts();

  float angle = ((float)pos * 360.0) / PULSES_PER_REV;
  while (angle < 0) angle += 360;
  while (angle >= 360) angle -= 360;

  //Serial.print(encoderPos);
  //Serial.print(",");
  //Serial.println(angle);

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
      //Serial.println("Target reached.");
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

    //Serial.print("Delta: ");
    //Serial.print(delta);
    //Serial.print("  Error: ");
    //Serial.print(error);
    //Serial.print("  Speed: ");
    //Serial.println(speed);
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
