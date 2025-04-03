#define IN1 6
#define IN2 5 
#define PULSES_PER_REV 784  
#define PULSES_EACH_MOT_REV 16
#define IDLE_STATE 0
#define INIT_STATE 1
#define MODEL_STATE 2
#define MODELED_STATE 3
#define RETURN_STATE 4
#define READY_STATE 5
#define FUNCTION_STATE 6
#define TAKEOFF_STATE 7
#define O2REF_TIME 1000

volatile long encoderPos = 0;
float ctrl_coeff = 1;
int rotate_counter = 0;
int state = IDLE_STATE;
unsigned long time_start = 0;
unsigned long init_interval = 0;
unsigned long O2_interval = 0;
unsigned long rev_start = 0;
unsigned long rev_end = 0;

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
    } else if (command.startsWith("S")){
      //record & model the wrist begins, will run as the IRS goes
      state = INIT_STATE;
    } else if (command.startsWith("T")){
      //record & model the wrist begins, will run as the IRS goes
      state = TAKEOFF_STATE;
    }
  }
  
  if (state == INIT_STATE){
    // begin the modeling process, assume cw is tighting, ccw is untighting
    analogWrite(IN1, 255);
    analogWrite(IN2, 0);
    // make the thing rotate for 1 to find how fast is it, also record the start time
    time_start = millis();
    rotate_counter = 0;
  } else if (state == MODELED_STATE) {
    // check how long does this case compare to others, then save the coefficient!
    ctrl_coeff = (float) O2_interval / O2REF_TIME;
    state = RETURN_STATE;
    // goes back a bit....
    analogWrite(IN1, 0);
    analogWrite(IN2, 255);
  } else if (state == READY_STATE) {
    // setup completed!!!
    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
    state = FUNCTION_STATE;
  } else if (state == TAKEOFF_STATE) {
    // return to the init setup for user to take off the device
    analogWrite(IN1, 0);
    analogWrite(IN2, 255);
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
  rotate_counter++;
  if (rotate_counter == PULSES_EACH_MOT_REV){
    rotate_counter = 0;
    if (state == INIT_STATE){
      // first rev completed record how long it took
      rev_start = millis();
      init_interval = rev_start - time_start;
      state = MODEL_STATE;
    } else if (state == MODEL_STATE) {
      // keep tighting till we reach 0.2 times the original speed
      rev_end = millis();
      unsigned long difference = rev_end - rev_start;
      if (abs(difference - 5 * init_interval) < 10) {
        // reach 0.2 point, record how long it took to reach this point
        O2_interval = rev_end - time_start;
        state = MODELED_STATE;
      }
      rev_start = rev_end;
    } else if (state == RETURN_STATE) {
      // keep going untight till we reach 0.5 (can change) times the original speed
      rev_end = millis();
      unsigned long difference = rev_end - rev_start;
      if (abs(difference - 2 * init_interval) < 10) {
        // reach backup place, go to ready state
        state = READY_STATE;
      }
      rev_start = rev_end;
    } else if (state == TAKEOFF_STATE){
      // keep untight till we have same speed as the original speed
      rev_end = millis();
      unsigned long difference = rev_end - rev_start;
      if (abs(difference - init_interval) < 10) {
        // reach backup place, go to ready state
        state = IDLE_STATE;
        analogWrite(IN1, 0);
        analogWrite(IN2, 0);
      }
      rev_start = rev_end;
    }
  }
}
