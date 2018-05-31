const int soundSensorPin = A0; // Piezo output
const int buzzerPin = 12; // Piezo output

const int Schwellenwert = 502;

const int volume;

void setup() 
{
  Serial.begin(9600);
  pinMode(buzzerPin, OUTPUT);
//  pinMode(soundSensorPin, INPUT);
}

void loop() 
{
//  digitalWrite(buzzerPin, HIGH);
  // Read Piezo ADC value in, and convert it to a voltage
  int volume = analogRead(soundSensorPin);
  Serial.println(volume); // Print the voltage.
  
  delay(10);

  if(volume<=Schwellenwert) {
     digitalWrite(buzzerPin, LOW);
  } else {
         digitalWrite(buzzerPin, HIGH);
  }
}
