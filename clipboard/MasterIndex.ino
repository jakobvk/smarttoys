#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>

// analogIn
 #include <driver/adc.h>

char* ssid = "Interface Werkstatt Guest";
char* password = "interfacelabor!";

char* code = "";

// Master

// Global copy of slave
#define NUMSLAVES 3
esp_now_peer_info_t slaves[NUMSLAVES] = {};
int SlaveCnt = 0;

#define CHANNEL 0
#define PRINTSCANRESULTS 0


// Pyramid
const int trigPin1=32;
const int echoPin1=35;

const int trigPin2=27;
const int echoPin2=26;

const int trigPin3=25;
const int echoPin3=33;//23!!!!!!!!! FALSCHE BESCHRIFTUNG AUF NODEMCU (GELBES BOARD)

const int lampPin=14;

//box

const int buzzerPin=12;
const int soundSensorPin=36;

const int soundTreshold=0.2;
//ball

const int vibrationPin=4;
const int vibrationSensorPin=32;
/////

WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    //Set device in STA mode to begin with
    WiFi.mode(WIFI_STA);
    Serial.println("ESPNow/Multi-Slave/Master Example");
    // This is the mac address of the Master in Station Mode
    Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());

    // Init ESPNow with a fallback logic
    InitESPNow();
    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
    // register for recieve also
    esp_now_register_recv_cb(OnDataRecv);

    //Pyramid
    pinMode(trigPin1, OUTPUT);
    pinMode(echoPin1, INPUT);
    pinMode(trigPin2, OUTPUT);
    pinMode(echoPin2, INPUT);

    pinMode(lampPin, OUTPUT);

    //Box
    pinMode(buzzerPin, OUTPUT);
    analogReadResolution(10);

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



// Init ESP Now with fallback
void InitESPNow() {
if (esp_now_init() == ESP_OK) {
  Serial.println("ESPNow Init Success");
  configDeviceAP();
}
else {
  Serial.println("ESPNow Init Failed");
  // Retry InitESPNow, add a counte and then restart?
  // InitESPNow();
  // or Simply Restart
  ESP.restart();
}
}

// Scan for slaves in AP mode
void ScanForSlave() {
int8_t scanResults = WiFi.scanNetworks();
//reset slaves
memset(slaves, 0, sizeof(slaves));
SlaveCnt = 0;
Serial.println("");
if (scanResults == 0) {
  Serial.println("No WiFi devices in AP Mode found");
} else {
  Serial.print("Found "); Serial.print(scanResults); Serial.println(" devices ");
  for (int i = 0; i < scanResults; ++i) {
    // Print SSID and RSSI for each device found
    String SSID = WiFi.SSID(i);
    int32_t RSSI = WiFi.RSSI(i);
    String BSSIDstr = WiFi.BSSIDstr(i);

    if (PRINTSCANRESULTS) {
      Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
    }
    delay(10);
    // Check if the current device starts with `Slave`
    if (SSID.indexOf("Slave") == 0) {
      // SSID of interest
      Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
      // Get BSSID => Mac Address of the Slave
      int mac[6];

      if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
        for (int ii = 0; ii < 6; ++ii ) {
          slaves[SlaveCnt].peer_addr[ii] = (uint8_t) mac[ii];
        }
      }
      slaves[SlaveCnt].channel = CHANNEL; // pick a channel
      slaves[SlaveCnt].encrypt = 0; // no encryption
      SlaveCnt++;
    }
  }
}

if (SlaveCnt > 0) {
  Serial.print(SlaveCnt); Serial.println(" Slave(s) found, processing..");
} else {
  Serial.println("No Slave Found, trying again.");
}

// clean up ram
WiFi.scanDelete();
}


// config AP SSID
void configDeviceAP() {
  String Prefix = "Master:";
  String Mac = WiFi.macAddress();
  String SSID = Prefix + Mac;
  String Password = "123456789";
  bool result = WiFi.softAP(SSID.c_str(), Password.c_str(), CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}

// Check if the slave is already paired with the master.
// If not, pair the slave with master
void manageSlave() {
  if (SlaveCnt > 0) {
    for (int i = 0; i < SlaveCnt; i++) {
      const esp_now_peer_info_t *peer = &slaves[i];
      const uint8_t *peer_addr = slaves[i].peer_addr;
      Serial.print("Processing: ");
      for (int ii = 0; ii < 6; ++ii ) {
        Serial.print((uint8_t) slaves[i].peer_addr[ii], HEX);
        if (ii != 5) Serial.print(":");
      }
      Serial.print(" Status: ");
      // check if the peer exists
      bool exists = esp_now_is_peer_exist(peer_addr);
      if (exists) {
        // Slave already paired.
        Serial.println("Already Paired");
      } else {
        // Slave not paired, attempt pair
        esp_err_t addStatus = esp_now_add_peer(peer);
        if (addStatus == ESP_OK) {
          // Pair success
          Serial.println("Pair success");
        } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
          // How did we get so far!!
          Serial.println("ESPNOW Not Init");
        } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
          Serial.println("Add Peer - Invalid Argument");
        } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
          Serial.println("Peer list full");
        } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
          Serial.println("Out of memory");
        } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
          Serial.println("Peer Exists");
        } else {
          Serial.println("Not sure what happened");
        }
        delay(100);
      }
    }
  } else {
    // No slave found to process
    Serial.println("No Slave found to process");
  }
}


// send data
void sendData(uint8_t data) {
//  data++;
for (int i = 0; i < SlaveCnt; i++) {
  const uint8_t *peer_addr = slaves[i].peer_addr;
  if (i == 0) { // print only for first slave
    Serial.print("Sending: ");
    Serial.println(data);
  }
  esp_err_t result = esp_now_send(peer_addr, &data, sizeof(data));
  Serial.print("Send Status: ");
  if (result == ESP_OK) {
    Serial.println("Success");
  } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW not Init.");
  } else if (result == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
  } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
    Serial.println("Internal Error");
  } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer not found.");
  } else {
    Serial.println("Not sure what happened");
  }
  delay(100);
}
}

// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
String s = WiFi.macAddress();
  Serial.print("MAC: "); Serial.println(s);
char macStr[18];
snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
         mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
Serial.print("Last Packet Sent to: "); Serial.println(macStr);
Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// // callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  Serial.print("Last Packet Recv Data: "); Serial.println(*data);
  Serial.println("");
}

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

         // TODO: find a more beautiful way!
         // char circleText[7] = "circle";
         // char boxText[4] = "box";
         // char pyramidText[8] = "pyramid";
         // char getText[6] = "GET /";
         //
         // char addder = getText + circleText;



        // SINGLE
         if (currentLine.endsWith("GET /circle1+circle2+circle3")) {
            code = "ccc";
         } else if (currentLine.endsWith("GET /pyramid1+pyramid2+pyramid3")) {
           code = "ppp";
         } else if (currentLine.endsWith("GET /box1+box2+box3")) {
            code = "bbb";
         } else if (currentLine.endsWith("GET /circle1+circle2+circle3")) {
            code = "ccc";
         }  else if (currentLine.endsWith("GET /pyramid1+pyramid2+box3")) {
            code = "ppb";
         }

       }
     }
     // close the connection:
     client.stop();
     Serial.println("Client Disconnected.");
   }
};

void loop(){
  // start Server
 serverFunction();

 // In the loop we scan for slave
 ScanForSlave();
 // If Slave is found, it would be populate in `slave` variable
 // We will check if `slave` is defined and then we proceed further
 if (SlaveCnt > 0) { // check if slave channel is defined
   // `slave` is defined
   // Add slave as peer if it has not been added already
   manageSlave();
   // pair success or already paired

 } else {
   // No slave found to process
 }

 // wait for 3seconds to run the logic again
 delay(1000);
// SINGLE functions
 if (code == "ppp") {
    Serial.println("PyramidFunctionBOTH"); // Print the voltage.
    // pyramidFunction("both");
    // 1 == KUGEL; 2 == PYRAMIDE; 3 == BOX;
    // 1 == INPUT; 2 == OUTPUT; 3 == BOTH;
    // Send data to device
    sendData(23);
  } else if (code == "bbb"){
    Serial.println("BOX MODE");
    sendData(13);
   // boxFunction("both");
 } else if (code == "ccc"){
   Serial.println("PyramidFunctionBOTH"); // Print the voltage.
   sendData(13);
  // ballFunction("both");
}
// TWOWAY functions
else if (code == "ppb"){
  // int pyramidPyramidInput = pyramidFunction("input");
  // if (pyramidPyramidInput) {
  //   // boxFunction("output");
  //   Serial.println("STARTING OUTPUT"); // Print the voltage.
  //   delay(1000);
  // }
 }  else if (code == "pyramid1+pyramid2+box3"){
  // int pyramidPyramidInput = pyramidFunction("input");
  // if (pyramidPyramidInput) {
  //   // ballFunction("output");
  //   Serial.println("STARTING OUTPUT"); // Print the voltage.
  //   delay(1000);
  // }
 } else {
        delay(500);
        Serial.println("NO function chosen or wrong code: ");
        Serial.println(code);
        Serial.println("sending 00 to stop all onging processes ");
        sendData(00);
  }
}
