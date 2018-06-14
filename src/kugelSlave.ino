/**
   ESPNOW - Basic communication - Slave
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and multiple ESP32 Slaves
   Description: This sketch consists of the code for the Slave module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/
   << This Device Slave >>
   Flow: Master
   Step 1 : ESPNow Init on Master and set it in STA mode
   Step 2 : Start scanning for Slave ESP32 (we have added a prefix of `slave` to the SSID of slave for an easy setup)
   Step 3 : Once found, add Slave as peer
   Step 4 : Register for send callback
   Step 5 : Start Transmitting data from Master to Slave(s)
   Flow: Slave
   Step 1 : ESPNow Init on Slave
   Step 2 : Update the SSID of Slave with a prefix of `slave`
   Step 3 : Set Slave in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor
   Note: Master and Slave have been defined to easily understand the setup.
         Based on the ESPNOW API, there is no concept of Master and Slave.
         Any devices can act as master or salve.
*/
// #include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

char* ssid = "Interface Werkstatt Guest";
char* password = "interfacelabor!";

const int ledPin = 14;

#define NUMSLAVES 3
esp_now_peer_info_t slaves[NUMSLAVES] = {};
int SlaveCnt = 0;

#define CHANNEL 1

// Ball

const int vibrationPin=13;
const int vibrationSensorPin=33;
const int threshold = 100;


// Logik
uint8_t messageCode = 00;
//

// Init ESP Now with fallback
void InitESPNow() {
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  String Prefix = "Slave:";
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

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);


  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.begin(115200);
  Serial.println("ESPNow/Basic/Slave Example");
  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP);



  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);

  scanNetwork();

  manageSlave();

}

uint8_t data = 33;

int sendData(const uint8_t *mac) {

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


  // int8_t scanResults = WiFi.scanNetworks();
  //
  // Serial.println("RESULTS");
  // Serial.println((uint8_t) scanResults);
//   // if (scanResults == 0) {
//   //   Serial.println("No WiFi devices in AP Mode found");
//   // } else {
//   //   Serial.print("Found "); Serial.print(scanResults); Serial.println(" devices ");
//   //   for (int i = 0; i < scanResults; ++i) {
//   //     // Print SSID and RSSI for each device found
//   //     String SSID = WiFi.SSID(i);
//   //     int32_t RSSI = WiFi.RSSI(i);
//   //     String BSSIDstr = WiFi.BSSIDstr(i);
//   //
//   //     // if (PRINTSCANRESULTS) {
//   //     //   Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
//   //     // }
//   //     delay(10);
//   //     // Check if the current device starts with `Slave`
//   //       // SSID of interest
//   //       Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
//   //       // Get BSSID => Mac Address of the Slave
//   //       int mac[6];
//   //
//   //       if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
//   //         for (int ii = 0; ii < 6; ++ii ) {
//   //           slaves[SlaveCnt].peer_addr[ii] = (uint8_t) mac[ii];
//   //         }
//   //       }
//   //       slaves[SlaveCnt].channel = CHANNEL; // pick a channel
//   //       slaves[SlaveCnt].encrypt = 0; // no encryption
//   //       SlaveCnt++;
//   //
//     // }
//   }
//
//   Serial.println("Trying to send to: ");
//   Serial.println((uint8_t) *mac);
//   // boolean canIFindPeer = esp_now_is_peer_exist(mac);
//   // Serial.println("IT IS: ");Serial.println(canIFindPeer);
//   esp_err_t canIFindPeer = esp_now_get_peer_num(0);
//   Serial.println("IT IS: ");Serial.println(canIFindPeer);
//
//   Serial.print("PEER: ");
//   if (canIFindPeer = ESP_OK) {
//     Serial.println("succeed");
//   } else if (canIFindPeer = ESP_ERR_ESPNOW_NOT_INIT) {
//     Serial.println("ESPNOW is not initialized");
//   } else if (canIFindPeer = ESP_ERR_ESPNOW_ARG) {
//     Serial.println("invalid argument");
//   };
//
//   // Serial.print(mac);
//       uint8_t dataR = 33;
// //       uint8_t senderAddress = (uint8_t)atoi(mac);
// // //      char tester = printf("%d", mac);
// //       Serial.println(senderAddress);
//     esp_err_t result = esp_now_send(mac, &dataR, sizeof(dataR));
//    Serial.print("Send Status: ");
//    if (result == ESP_OK) {
//      Serial.println("Success");
//    } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
//      // How did we get so far!!
//      Serial.println("ESPNOW not Init.");
//    } else if (result == ESP_ERR_ESPNOW_ARG) {
//      Serial.println("Invalid Argument");
//    } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
//      Serial.println("Internal Error");
//    } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
//      Serial.println("ESP_ERR_ESPNOW_NO_MEM");
//    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
//      Serial.println("Peer not found.");
//    } else {
//      Serial.println("Not sure what happened");
//    }
//    delay(100);
 }
// // callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  Serial.print("Last Packet Recv Data: "); Serial.println(*data);
  Serial.println("");
  sendData(mac_addr);

  // TODO: remoe this after test environment
  // if (*data) {
  //       digitalWrite(ledPin, HIGH);
  //   delay(200);
  //   digitalWrite(ledPin, LOW);
  //     delay(200);
  // }
  messageCode = *data;
  //
}

void scanNetwork () {
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

      if (true) {
        Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
      }
      delay(10);
      // Check if the current device starts with `Slave`
      if (SSID.indexOf("Master") == 0) {
        Serial.print("found a Master");
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
}}

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

//
// THE FUNCTION PART
//


int ballFunction(char* parameter) {
  digitalWrite(vibrationPin, LOW);
  bool returner = false;
  if (parameter == "input") {
    // Read Piezo ADC value in, and convert it to a voltage
    int piezoADC = analogRead(vibrationSensorPin);
    float piezoV = piezoADC / 1023.0 * 5.0;
    Serial.println(piezoV); // Print the voltage.
    if (piezoV == 0.01 || piezoV > 0.01) {
        digitalWrite(vibrationPin, HIGH);
        returner = true;
        Serial.println("INPUT SENSORED");
        return returner;
    }
  } else if (parameter == "output") {
    digitalWrite(vibrationPin, HIGH);
    delay(3000);
    return true;
  } else if (parameter == "both") {
    // Read Piezo ADC value in, and convert it to a voltage
    int piezoADC = analogRead(vibrationSensorPin);
    float piezoV = piezoADC / 1023.0 * 5.0;
    Serial.println(piezoV); // Print the voltage.
    if (piezoV == 0.01 || piezoV > 0.01) {
        digitalWrite(vibrationPin, HIGH);
    }
  }
  delay(100);
}

void loop() {
  delay(200);
  if (messageCode == 11) {
    // input mode
    int triggerMessage = ballFunction("input"); // ball function should return true

    if (triggerMessage) {
      sendData(&messageCode);  // send return message
      delay(200);
      messageCode = 0; // stop Input function
    }
  } else if (messageCode == 12){
    int triggerMessage = ballFunction("output"); // ball function should return true
    sendData(&messageCode);  // send return message
    delay(200);
    messageCode = 0; // stop output function
  } else if (messageCode == 13) {
    // ballFunction("BOTH");
      Serial.print("Message Code: ");Serial.print(messageCode);

      ballFunction("both");
      // digitalWrite(ledPin, HIGH);
      // delay(200);
      // digitalWrite(ledPin, LOW);
      // delay(200);
  } else {
    // Chill
  }
}
