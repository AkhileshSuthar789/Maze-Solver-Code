// ================= MOTOR PINS =================
// Motor A (Left)
#define PWMA 6
#define AIN1 1   // Note: Shared with Serial TX. 
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

// ================= SETTINGS =================
const int BASE_SPEED = 160;    // Higher base power for the weak motor
const int TURN_SPEED = 160;
const int THRESHOLD = 500; 
const int WALL_STOP_DIST = 10; 

/* CALIBRATION: 
  If the robot still veers LEFT: Increase RIGHT_OFFSET (slows down the right wheel).
  If the robot veers RIGHT: Decrease RIGHT_OFFSET.
*/
const int RIGHT_OFFSET = 30;   // Adjusted for higher base speed

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

// ================= MAIN LOOP =================
void loop() {
  long distance = getDistance();

  if (distance > 0 && distance < WALL_STOP_DIST) {
    stopMotors();
    delay(200);
    uTurn(); 
  } 
  else {
    mazeLogic();
  }
}

// ================= MAZE LOGIC =================
void mazeLogic() {
  int s1 = analogRead(S1);
  int s2 = analogRead(S2);
  int s3 = analogRead(S3);
  int s4 = analogRead(S4);

  bool L  = (s1 < THRESHOLD); 
  bool LC = (s2 < THRESHOLD); 
  bool RC = (s3 < THRESHOLD); 
  bool R  = (s4 < THRESHOLD); 

  if (L && LC && RC && R) {
    uTurn();
  }
  else if ((LC && RC) || (L && R)) {
    moveForward();
  }
  else if (L || LC) {
    turnLeft();
  }
  else if (R || RC) {
    turnRight();
  }
  else if (RC && LC && R) {
    turnLeft();
  }
  else if (RC && LC && L) {
    turnLeft();
  }
  else if (R || L) {
    moveForward();
  }
  else {
    moveForward();
  }
}

// ================= MOTOR FUNCTIONS =================

void moveForward() {
  // Left Motor (A) - Full Power
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, BASE_SPEED);

  // Right Motor (B) - Reduced to match Left
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, BASE_SPEED - RIGHT_OFFSET);
}

void turnLeft() {
  // Pivot Left
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, TURN_SPEED);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, TURN_SPEED - RIGHT_OFFSET);
}

void turnRight() {
  // Pivot Right
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, TURN_SPEED);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, TURN_SPEED - RIGHT_OFFSET);
}

void uTurn() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, TURN_SPEED);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, TURN_SPEED - RIGHT_OFFSET);

  delay(800); 
  stopMotors();
}

void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
}

// ================= SENSORS =================

long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 20000);
  if (duration == 0) return 0;
  return duration * 0.034 / 2;
}