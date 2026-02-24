// Maze Solver Robot - Left Hand Wall Follower
// Ultrasonic for front, IR for left/right sides
// Smooth ramp + kick-start + encoder veer correction
// Added min PWM for left motor to fix high startup threshold

// ────────────────────────────────────────────────
// Pins
// ────────────────────────────────────────────────

// Motors (TB6612)
#define PWMA 6     // Left
#define AIN1 1
#define AIN2 8
#define PWMB 7     // Right
#define BIN1 4
#define BIN2 5

// Encoders
#define ENCODER_LEFT_A  3
#define ENCODER_LEFT_B  10
#define ENCODER_RIGHT_A 2
#define ENCODER_RIGHT_B 9

// Sensors
#define IR_LEFT     A1
#define IR_RIGHT    A3
#define WALL_THRESHOLD  450   // Tune: analogRead > this = wall on side

#define ULTRASONIC_TRIGGER 11
#define ULTRASONIC_ECHO    12
#define FRONT_SAFE_CM      12   // Front clear if > this distance (cm)

// Speeds & Tuning
#define BASE_SPEED     130    // Main forward (100-160 range – lower = smoother)
#define MIN_PWM_LEFT   130    // Min PWM for left motor startup (based on your tests)
#define LEFT_OFFSET    35     // Extra PWM to left to fight right veer & high threshold (tune 30-45)
#define TURN_SPEED     110
#define RAMP_STEP      8      // Acceleration smoothness (smaller = gentler)
#define KICK_PWM       200
#define KICK_MS        70     // Gentle kick when starting

// Turn calibration (test & adjust!)
#define PULSES_PER_90  220    // Pulses for accurate 90° pivot

volatile long leftCount  = 0;
volatile long rightCount = 0;

int currentLeftSpeed  = 0;
int currentRightSpeed = 0;

void leftISR()  { leftCount++;  }
void rightISR() { rightCount++; }

void setup() {
  // Motor pins
  pinMode(PWMA, OUTPUT); pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT); pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);

  // Encoders
  pinMode(ENCODER_LEFT_A,  INPUT_PULLUP);
  pinMode(ENCODER_LEFT_B,  INPUT_PULLUP);
  pinMode(ENCODER_RIGHT_A, INPUT_PULLUP);
  pinMode(ENCODER_RIGHT_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT_A),  leftISR,  RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT_A), rightISR, RISING);

  // Sensors
  pinMode(IR_LEFT,  INPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(ULTRASONIC_TRIGGER, OUTPUT);
  pinMode(ULTRASONIC_ECHO,    INPUT);

  stopMotors();
  delay(1500);  // Startup settle time
}

void loop() {
  bool leftWall  = analogRead(IR_LEFT)  > WALL_THRESHOLD;
  bool rightWall = analogRead(IR_RIGHT) > WALL_THRESHOLD;
  bool frontWall = getDistance() < FRONT_SAFE_CM;

  // Left-hand rule decisions
  if (!leftWall) {
    // Left open → turn left
    turnLeft90();
  }
  else if (frontWall) {
    // Front blocked → turn right
    turnRight90();
  }
  else {
    // Follow left wall → forward with smoothing
    moveForwardSmooth();
  }

  // Dead-end fallback (all sides blocked except back)
  if (frontWall && leftWall && rightWall) {
    turn180();
  }

  delay(15);  // Loop stability
}

// ────────────────────────────────────────────────
// Movement Functions (smooth & corrected)
// ────────────────────────────────────────────────

void moveForwardSmooth() {
  int targetLeft  = max(BASE_SPEED + LEFT_OFFSET, MIN_PWM_LEFT);  // Clamp to min
  int targetRight = BASE_SPEED;

  // Gentle ramp acceleration/deceleration
  if (currentLeftSpeed < targetLeft) {
    currentLeftSpeed += RAMP_STEP;
    if (currentLeftSpeed > targetLeft) currentLeftSpeed = targetLeft;
  } else if (currentLeftSpeed > targetLeft) {
    currentLeftSpeed -= RAMP_STEP;
  }

  if (currentRightSpeed < targetRight) {
    currentRightSpeed += RAMP_STEP;
    if (currentRightSpeed > targetRight) currentRightSpeed = targetRight;
  } else if (currentRightSpeed > targetRight) {
    currentRightSpeed -= RAMP_STEP;
  }

  // Very gentle kick only when almost stopped
  if (currentLeftSpeed < 40 && currentLeftSpeed > 0) {
    setMotorLeft(1, KICK_PWM);
    setMotorRight(1, KICK_PWM);
    delay(KICK_MS);
  }

  setMotorLeft(1, currentLeftSpeed);
  setMotorRight(1, currentRightSpeed);
}

void turnLeft90() {
  stopMotors(); delay(100);
  resetEncoders();
  currentLeftSpeed = currentRightSpeed = 0;  // Reset ramp

  while (rightCount < PULSES_PER_90) {
    setMotorLeft(-1, max(TURN_SPEED, MIN_PWM_LEFT));   // Left back with min clamp
    setMotorRight(1, TURN_SPEED);   // Right forward
  }
  stopMotors(); delay(150);
}

void turnRight90() {
  stopMotors(); delay(100);
  resetEncoders();
  currentLeftSpeed = currentRightSpeed = 0;

  while (leftCount < PULSES_PER_90) {
    setMotorLeft(1, max(TURN_SPEED, MIN_PWM_LEFT));    // Left forward with min clamp
    setMotorRight(-1, TURN_SPEED);
  }
  stopMotors(); delay(150);
}

void turn180() {
  stopMotors(); delay(100);
  resetEncoders();
  currentLeftSpeed = currentRightSpeed = 0;

  while (leftCount < (2 * PULSES_PER_90)) {
    setMotorLeft(1, max(TURN_SPEED, MIN_PWM_LEFT));
    setMotorRight(-1, TURN_SPEED);
  }
  stopMotors(); delay(200);
}

void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
  currentLeftSpeed = currentRightSpeed = 0;
}

void setMotorLeft(int dir, int speed) {
  speed = constrain(speed, 0, 255);
  if (dir == 1)      { digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);  }
  else if (dir == -1){ digitalWrite(AIN1, LOW);  digitalWrite(AIN2, HIGH); }
  else               { digitalWrite(AIN1, LOW);  digitalWrite(AIN2, LOW);  }
  analogWrite(PWMA, speed);
}

void setMotorRight(int dir, int speed) {
  speed = constrain(speed, 0, 255);
  if (dir == 1)      { digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);  }
  else if (dir == -1){ digitalWrite(BIN1, LOW);  digitalWrite(BIN2, HIGH); }
  else               { digitalWrite(BIN1, LOW);  digitalWrite(BIN2, LOW);  }
  analogWrite(PWMB, speed);
}

long getDistance() {
  digitalWrite(ULTRASONIC_TRIGGER, LOW);
  delayMicroseconds(3);
  digitalWrite(ULTRASONIC_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIGGER, LOW);
  long duration = pulseIn(ULTRASONIC_ECHO, HIGH, 30000);  // timeout ~5m
  if (duration == 0) return 999;  // error → assume far
  return duration * 0.034 / 2;
}

void resetEncoders() {
  leftCount = rightCount = 0;
}