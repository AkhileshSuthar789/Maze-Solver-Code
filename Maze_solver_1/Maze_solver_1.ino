// ================= SENSOR CHECK =================
// IR Sensors
#define S1 A7  // Left Outer
#define S2 A6  // Left Center
#define S3 A3  // Right Center
#define S4 A2  // Right Outer

// Ultrasonic
#define TRIG A4
#define ECHO A5

void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  Serial.println("Sensor Check Started - Values updating every 500ms");
}

void loop() {
  // Read IR
  int s1 = analogRead(S1);
  int s2 = analogRead(S2);
  int s3 = analogRead(S3);
  int s4 = analogRead(S4);

  // Read Ultrasonic
  long distance = getDistance();

  // Print
  Serial.print("IR Left Outer (S1): "); Serial.print(s1);
  Serial.print(" | Left Center (S2): "); Serial.print(s2);
  Serial.print(" | Right Center (S3): "); Serial.print(s3);
  Serial.print(" | Right Outer (S4): "); Serial.println(s4);
  Serial.print("Ultrasonic Distance: "); Serial.print(distance); Serial.println(" cm");

  delay(500);  // Update rate
}

long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH, 30000);
  if (duration == 0) return 0;
  return duration * 0.034 / 2;
}