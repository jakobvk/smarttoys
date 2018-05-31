const int PIEZO_PIN = A0; // Piezo output
const int VIBRATION_PIN = 7; // Piezo output


void setup() 
{
  Serial.begin(9600);
  pinMode(VIBRATION_PIN, OUTPUT);
}

void loop() 
{
  digitalWrite(VIBRATION_PIN, LOW);
  // Read Piezo ADC value in, and convert it to a voltage
  int piezoADC = analogRead(PIEZO_PIN);
  float piezoV = piezoADC / 1023.0 * 5.0;
  Serial.println(piezoV); // Print the voltage.
  if (piezoV == 0.01 || piezoV > 0.01) {
      digitalWrite(VIBRATION_PIN, HIGH);
  }
  
  delay(100);
}
