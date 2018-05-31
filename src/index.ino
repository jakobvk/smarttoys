#include <WiFi.h>
#include <Wire.h>
// analogIn
// #include <driver/adc.h>

char* ssid     = "Ingo Lochmahr";
char* password = "Pforzheim030";

char* code = "";

// Pyramid
const int trigPin1=32;
const int echoPin1=35;

const int trigPin2=27;
const int echoPin2=26;

const int trigPin3=25;
const int echoPin3=33;

const int lampPin=21;

//box

const int buzzerPin=2;
const int soundSensorPin=34;

const int soundTreshold =502;
//ball

const int vibrationPin=4;
const int vibrationSensorPin=34;
/////

WiFiServer server(80);

void setup() {
    Serial.begin(9600);

    //Pyramid
    pinMode(trigPin1, OUTPUT);
    pinMode(echoPin1, INPUT);
    pinMode(trigPin2, OUTPUT);
    pinMode(echoPin2, INPUT);

    pinMode(lampPin, OUTPUT);

    //Box
    pinMode(buzzerPin, OUTPUT);

    //circle
    pinMode(vibrationPin, OUTPUT);

    //

    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    server.begin();

}

int value = 0;

int serverFunction() {
  WiFiClient client = server.available();   // listen for incoming clients

   if (client) {                             // if you get a client,
     Serial.println("New Client.");           // print a message out the serial port
     String currentLine = "";                // make a String to hold incoming data from the client
     while (client.connected()) {            // loop while the client's connected
       if (client.available()) {             // if there's bytes to read from the client,
         char c = client.read();             // read a byte, then
         Serial.write(c);                    // print it out the serial monitor
         if (c == '\n') {                    // if the byte is a newline character

           // if the current line is blank, you got two newline characters in a row.
           // that's the end of the client HTTP request, so send a response:
           if (currentLine.length() == 0) {
             // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
             // and a content-type so the client knows what's coming, then a blank line:
             client.println("HTTP/1.1 200 OK");
             client.println("Content-type:text/html");
             client.println();

             // the content of the HTTP response follows the header:
             // client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
             // client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

             client.print("<script language='JavaScript'> document.addEventListener('DOMContentLoaded', function(){/*/// GET FUNCTION */ var HttpClient=function(){this.get=function(aUrl, aCallback){var anHttpRequest=new XMLHttpRequest(); anHttpRequest.onreadystatechange=function(){if (anHttpRequest.readyState==4 && anHttpRequest.status==200); aCallback(anHttpRequest.responseText);}; anHttpRequest.open( 'GET', aUrl, true ); anHttpRequest.send( null );};}; console.log('loaded'); var buttons=['circle1', 'circle2', 'circle3', 'box1', 'box2', 'box3', 'pyramid1', 'pyramid2', 'pyramid3']; var notChosen=['circle1', 'circle2', 'circle3', 'box1', 'box2', 'box3', 'pyramid1', 'pyramid2', 'pyramid3']; var chosen1=[]; var chosen2=[]; var chosen3=[]; buttons.forEach(function(button){document.getElementById(button).addEventListener('click', function(){/*/// add choice */ if(button==='circle1' || button==='box1' || button==='pyramid1'){chosen1.forEach(function(thing){document.getElementById(thing).classList.remove('active');}); document.getElementById(button).classList.toggle('active'); chosen1=[]; chosen1.push(button); notChosen.splice(button);}else if(button==='circle2' || button==='box2' || button==='pyramid2'){chosen2.forEach(function(thing){document.getElementById(thing).classList.remove('active');}); document.getElementById(button).classList.toggle('active'); chosen2=[]; chosen2.push(button); notChosen.splice(button);}else if(button==='circle3' || button==='box3' || button==='pyramid3'){chosen3.forEach(function(thing){document.getElementById(thing).classList.remove('active');}); document.getElementById(button).classList.toggle('active'); chosen3=[]; chosen3.push(button); notChosen.splice(button);}; /*/// function called when things are chosen */ if(chosen1.length > 0 && chosen2.length > 0 && chosen3.length > 0){var getString='/' + chosen1[0] + '+' + chosen2[0] + '+' + chosen3[0]; var client=new HttpClient(); client.get(getString, function(response){/*/// do something with response */});};});});}); </script> <div class='flex-container'> <div class='row'> <div class='circle' id='circle1'> <span></span> </div><div class='circle' id='circle2'> <span></span> </div><div class='circle' id='circle3'> <span></span> </div></div><div class='row'> <div class='box' id='box1'> <span></span> </div><div class='box' id='box2'> <span></span> </div><div class='box' id='box3'> <span></span> </div></div><div class='row'> <div class='pyramid' id='pyramid1'> <span class='border'></span> </div><div class='pyramid' id='pyramid2'> <span class='border'></span> </div><div class='pyramid' id='pyramid3'> <span class='border'></span> </div></div></div><style media='screen'> .container{display: flex; flex-flow: column; justify-content: space-around;}.row{display: flex; flex-flow: row; justify-content: space-around;}.row :nth-child(3){border-left: 10px black solid;}.circle, .pyramid, .box{display: flex; justify-content: center; align-items: center; height: 33vh; width: 33vw;}.pyramid span{width: 0; height: 0; border-style: solid; border-width: 0 95px 190px 95px; border-color: transparent transparent #bbb transparent;}.box span{height: 180px; width: 180px; background-color: #bbb; display: inline-block;}.circle span{height: 200px; width: 200px; background-color: #bbb; border-radius: 50%; display: inline-block;}.active span{background-color: #000; border-color: #000}.active .border{background-color: transparent; border-color: transparent transparent black transparent;}</style>");
             // The HTTP response ends with another blank line:
             client.println();
             // break out of the while loop:
             break;
           } else {    // if you got a newline, then clear currentLine:
             currentLine = "";
           }
         } else if (c != '\r') {  // if you got anything else but a carriage return character,
           currentLine += c;      // add it to the end of the currentLine
         }

         // Check to see if the client request was "GET /H" or "GET /L":
         if (currentLine.endsWith("GET /circle1+box2+pyramid3")) {
           Serial.println();
           Serial.println();
           Serial.println("GOTCHA kreis1+box2+pyramid3");
           code = "kreis1+box2+pyramid3";
 //          digitalWrite(12, HIGH);               // GET /H turns the LED on
            Serial.println(code);
         }
         if (currentLine.endsWith("GET /L")) {
 //          digitalWrite(12, LOW);                // GET /L turns the LED off
         }
       }
     }
     // close the connection:
     client.stop();
     Serial.println("Client Disconnected.");
   }
};

int pyramidFunction() {
  long duration1, distance1;
  digitalWrite(trigPin1, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  distance1 = (duration1/2) / 29.1;

   if (distance1 >= 500 || distance1 <= 0){
    Serial.println("Sensor1 Out of range");

    digitalWrite(lampPin, LOW);

  }
  else {
    Serial.print ( "Sensor1  ");
    Serial.print ( distance1);
    Serial.println("cm");

    digitalWrite(lampPin, HIGH);

  }
  delay(100);
long duration2, distance2;
  digitalWrite(trigPin2, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin2, LOW);
  duration2 = pulseIn(echoPin2, HIGH);
  distance2= (duration2/2) / 29.1;

   if (distance2 >= 500 || distance2 <= 0){
    Serial.println("Sensor2 Out of range");
  }
  else {
    Serial.print("Sensor2  ");
    Serial.print(distance2);
    Serial.println("cm");
  }
  delay(100);
};

// BOX FUNCTIONS
int boxFunction() {
  int soundVolume = analogRead(soundSensorPin);
  Serial.println(soundVolume); // Print the voltage.

  delay(10);

  if(soundVolume<=soundTreshold) {
     digitalWrite(buzzerPin, LOW);
  } else {
         digitalWrite(buzzerPin, HIGH);
  }
}

// BALL FUNCTIONS

int ballFunction() {
  digitalWrite(vibrationPin, LOW);
  // Read Piezo ADC value in, and convert it to a voltage
  int piezoADC = analogRead(vibrationSensorPin);
  float piezoV = piezoADC / 1023.0 * 5.0;
  Serial.println(piezoV); // Print the voltage.
  if (piezoV == 0.01 || piezoV > 0.01) {
      digitalWrite(vibrationPin, HIGH);
  }

  delay(100);
}

void loop(){
 serverFunction();
 if (code == "pyramid1+pyramid2+pyramid3") {
    pyramidFunction();
  } else if (code == "box1+box2+box3"){
   boxFunction();
 };
}
