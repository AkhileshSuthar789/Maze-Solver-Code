// Maze Solver - Left Hand Wall Follower (IR only, no ultrasonic)
// Encoder correction for straight line + kick-start for low speed

// Motor pins (TB6612)
#define PWMA 6     // Left motor PWM
#define AIN1 1
#define AIN2 8
#define PWMB 7     // Right motor PWM
#define BIN1 4
#define BIN2 5

// Encoders
#define ENCODER_LEFT_A 3
#define ENCODER_LEFT_B 10
#define ENCODER_RIGHT_A 2
#define ENCODER_RIGHT_B 9

// IR sensors (analog) - adjust pins and threshold for your setup
#define IR_LEFT   A1
#define IR_FRONT  A2
#define IR_RIGHT  A3
#define WALL_THRESHOLD  400   // Tune this: higher value = wall detected (test with Serial if needed)

// Speeds
#define BASE_SPEED      140   // Main forward speed (increase if too slow)
#define LEFT_OFFSET     20    // Extra PWM for left motor to correct right veer (start with 15-30, tune!)
#define TURN_SPEED      140
#define KICK_PWM        150   // Brief full power to start moving
#define KICK_TIME_MS    80    // 60-120 ms kick duration

// Encoder calibration for turns (test this!)
#define PULSES_PER_90   220   // Pulses for ~90° pivot - calibrate by counting in test code

volatile long leftCount  = 0;
volatile long rightCount = 0;

void leftISR()  { leftCount++;  }
void rightISR() { rightCount++; }

void setup() {
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(ENCODER_LEFT_A,  INPUT_PULLUP);
  pinMode(ENCODER_LEFT_B,  INPUT_PULLUP);
  pinMode(ENCODER_RIGHT_A, INPUT_PULLUP);
  pinMode(ENCODER_RIGHT_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT_A),  leftISR,  RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT_A), rightISR, RISING);

  pinMode(IR_LEFT,  INPUT);
  pinMode(IR_FRONT, INPUT);
  pinMode(IR_RIGHT, INPUT);

  stopMotors();
  delay(1500);  // Give time to place robot
}

void loop() {
  bool hasLeftWall   = analogRead(IR_LEFT)  > WALL_THRESHOLD;
  bool hasFrontWall  = analogRead(IR_FRONT) > WALL_THRESHOLD;
  bool hasRightWall  = analogRead(IR_RIGHT) > WALL_THRESHOLD;

  // Left-hand wall follower logic
  if (!hasLeftWall) {
    // Left open → turn left to follow new wall
    kickStart();
    turnLeft90();
  }
  else if (hasFrontWall) {
    // Front blocked → turn right
    kickStart();
    turnRight90();
  }
  else {
    // Wall on left, front open → go forward with correction
    moveForwardCorrected();
  }

  // Optional: Dead-end check (all blocked except back) → turn 180°
  if (hasFrontWall && hasLeftWall && hasRightWall) {
    kickStart();
    turn180();
  }

  delay(20);  // Small loop delay for sensor stability
}

// ────────────────────────────────────────────────
// Movement with kick-start and veer correction
// ────────────────────────────────────────────────

void moveForwardCorrected() {
  // Kick start if stopped recently (helps overcome friction)
  kickStart();

  // Base + offset to left motor to fight right veer
  setMotorLeft(1, BASE_SPEED + LEFT_OFFSET);
  setMotorRight(1, BASE_SPEED);
}

void kickStart() {
  // Brief full power pulse to start both motors reliably
  setMotorLeft(1, KICK_PWM);
  setMotorRight(1, KICK_PWM);
  delay(KICK_TIME_MS);
}

void turnLeft90() {
  stopMotors();
  resetEncoders();
  while (rightCount < PULSES_PER_90) {
    setMotorLeft(-1, TURN_SPEED);   // Left backward
    setMotorRight(1, TURN_SPEED);   // Right forward
  }
  stopMotors();
  delay(200);  // Short pause after turn
}

void turnRight90() {
  stopMotors();
  resetEncoders();
  while (leftCount < PULSES_PER_90) {
    setMotorLeft(1, TURN_SPEED);    // Left forward
    setMotorRight(-1, TURN_SPEED);  // Right backward
  }
  stopMotors();
  delay(200);
}

void turn180() {
  stopMotors();
  resetEncoders();
  while (leftCount < (2 * PULSES_PER_90)) {
    setMotorLeft(1, TURN_SPEED);
    setMotorRight(-1, TURN_SPEED);
  }
  stopMotors();
  delay(300);
}

void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}

void setMotorLeft(int dir, int speed) {
  if (dir == 1) {
    digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
  } else if (dir == -1) {
    digitalWrite(AIN1, LOW);  digitalWrite(AIN2, HIGH);
  } else {
    digitalWrite(AIN1, LOW);  digitalWrite(AIN2, LOW);
  }
  analogWrite(PWMA, constrain(speed, 0, 255));
}

void setMotorRight(int dir, int speed) {
  if (dir == 1) {
    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
  } else if (dir == -1) {
    digitalWrite(BIN1, LOW);  digitalWrite(BIN2, HIGH);
  } else {
    digitalWrite(BIN1, LOW);  digitalWrite(BIN2, LOW);
  }
  analogWrite(PWMB, constrain(speed, 0, 255));
}

void resetEncoders() {
  leftCount = 0;
  rightCount = 0;
}