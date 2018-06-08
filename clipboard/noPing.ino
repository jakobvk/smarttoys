
#define TRIGGER 16
#define ECHO 14
#define LIGHT 21
long duration;
long distance;

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(LIGHT, OUTPUT);
}

void loop() {
  digitalWrite(LIGHT, LOW);

  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIGGER, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration/2) / 29.1;

  if (distance <=4) {
    Serial.println("to close. ");
  } else if (distance >=30) {
    Serial.println("to far. ");Serial.println(distance);
  }
  else {
    digitalWrite(LIGHT, HIGH);
    Serial.println("Centimeters recorded:");Serial.println(distance);
  };

  delay(100);
}
