// ================= MOTOR PINS =================

// Motor A (Left)
#define PWMA 6
#define AIN1 1   // Note: Pin 1 is Serial TX. If upload fails, move to Pin 2 or 3.
#define AIN2 8

// Motor B (Right)
#define PWMB 7
#define BIN1 4
#define BIN2 5

// ================= IR SENSORS =================
#define S1 A7
#define S2 A6
#define S3 A3
#define S4 A2

// ================= ULTRASONIC =================
#define TRIG A4
#define ECHO A5

// ================= SETUP =================
void setup() {
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
}

// ================= LOOP =================
void loop() {
  long distance = getDistance();

  // Obstacle detection
  if (distance < 10 && distance > 0) {
    stopMotors();
    delay(100);
    turnRight(); 
  } 
  else {
    simpleLineFollow();
  }
}

// ================= LINE FOLLOWING & LOGIC =================
void simpleLineFollow() {
  int s1 = analogRead(S1);
  int s2 = analogRead(S2);
  int s3 = analogRead(S3);
  int s4 = analogRead(S4);

  int threshold = 500;

  bool L  = (s1 < threshold); // Left Outer
  bool LC = (s2 < threshold); // Left Center
  bool RC = (s3 < threshold); // Right Center
  bool R  = (s4 < threshold); // Right Outer

  // 1. ALL SENSORS DETECTED -> U-TURN
  if (L && LC && RC && R) {
    uTurn();
  }
  // 2. CENTER LINE OR BOTH SIDE WALLS -> FORWARD
  else if ((LC && RC) || (L && R)) {
    moveForward();
  }
  // 3. LEFT SIDE ACTIVE -> TURN LEFT
  else if (L || LC) {
    turnLeft();
  }
  // 4. RIGHT SIDE ACTIVE -> TURN RIGHT
  else if (R || RC) {
    turnRight();
  }
  // 5. NO LINE/WALLS -> STOP
  else {
    stopMotors();
  }
}

// ================= MOTOR FUNCTIONS =================

void moveForward() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, 150);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, 150);
}

void turnLeft() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, 120);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, 150);
}

void turnRight() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, 150);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, 120);
}

void uTurn() {
  // Spin in place (Left motor forward, Right motor backward)
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, 160);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, 160);

  delay(800); // Adjust this value until the robot turns exactly 180 degrees
  stopMotors();
  delay(100);
}

void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
}

// ================= ULTRASONIC =================
long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 20000);
  long distance = duration * 0.034 / 2;

  return distance;
}