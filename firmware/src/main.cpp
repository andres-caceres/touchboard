#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
//#include <WebServer.h>
#include <Wire.h>
#include <MPR121.h>
#include <MPR121_Datastream.h>
#include <SD.h>
#include <SPI.h>

#define MPR121_ADDR 0x5A
#define MPR121_INT 4

#ifndef _BV
#define _BV(x) (1 << x)
#endif
  
// MPR121 datastream behaviour constants
const bool MPR121_DATASTREAM_ENABLE = false;

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

//WebServer server(80); // Create a web server on port 80
WiFiUDP udp;
const int udpPort = 1234; // Port to send UDP packets

void handleRoot();
void handleSave();

void setupSensors();
void setupWifi();
void setupUDPserver();
void setupWebserver();

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);

  // Set up the serial port
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Starting...");

  // Set up the SD card
  //setupSDCard();
  
  // Set up the MPR121 sensor
  setupSensors();
  
  // Set up the WiFi
  setupWifi();
  
  // Set up the UDP server
  setupUDPserver();
  
  // Set up the web server
  //setupWebserver();

}

void loop() {
    
  MPR121.updateAll();

  if (MPR121_DATASTREAM_ENABLE) {
    MPR121_Datastream.update();
  }
  //server.handleClient(); // Handle incoming client requests
  
  const char *init_msg = "MPR121 Datastream";
  uint16_t data = MPR121.getTouchData();
  String message = String(data, HEX);

  // Broadcast the UDP packet
  udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
  udp.print(message); // Use udp.print() instead of udp.write()
  udp.endPacket();
  Serial.println(message);
  delay(5);
}

void setupWifi() {
  // Set up the Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void setupUDPserver() {
  // Set up the UDP server
  udp.begin(udpPort);
  Serial.println("UDP server started on port " + String(udpPort));
}
/*
void setupWebserver() {
  // Start the server
  server.on("/", handleRoot); // Handle the root URL
  server.begin();
  Serial.println("HTTP server started");
  server.on("/save", HTTP_POST, handleSave);
}
*/
void setupSensors() {
  // Set up the MPR121 sensor
  if (!MPR121.begin(MPR121_ADDR)) {
    Serial.println("error setting up MPR121");
    switch (MPR121.getError()) {
      case NO_ERROR:
        Serial.println("no error");
        break;
      case ADDRESS_UNKNOWN:
        Serial.println("incorrect address");
        break;
      case READBACK_FAIL:
        Serial.println("readback failure");
        break;
      case OVERCURRENT_FLAG:
        Serial.println("overcurrent on REXT pin");
        break;
      case OUT_OF_RANGE:
        Serial.println("electrode out of range");
        break;
      case NOT_INITED:
        Serial.println("not initialised");
        break;
      default:
        Serial.println("unknown error");
        break;
    }
    while (1);
  }

  MPR121.setInterruptPin(MPR121_INT);

  if (MPR121_DATASTREAM_ENABLE) {
    MPR121.restoreSavedThresholds();
    MPR121_Datastream.begin(&Serial);
  } else {
    MPR121.setTouchThreshold(40);  // this is the touch threshold - setting it low makes it more like a proximity trigger, default value is 40 for touch
    MPR121.setReleaseThreshold(20);  // this is the release threshold - must ALWAYS be smaller than the touch threshold, default value is 20 for touch
  }

  MPR121.setFFI(FFI_10);
  MPR121.setSFI(SFI_10);
  MPR121.setGlobalCDT(CDT_4US);  // reasonable for larger capacitances
  
  digitalWrite(LED_BUILTIN, HIGH);  // switch on user LED while auto calibrating electrodes
  delay(1000);
  MPR121.autoSetElectrodes();  // autoset all electrode settings
  digitalWrite(LED_BUILTIN, LOW);
}

void setupSDCard() {
  // Set up the SD card
  if (!SD.begin(15)) {
    Serial.println("error initializing SD card");
    while (1);
  }
}

/*
// Function to handle root URL
void handleRoot() {
    String html = "<h1>ESP32 Configuration</h1>";
    html += "<form action='/save' method='POST'>";
    html += "Parameter: <input type='text' name='param'><br>";
    html += "<input type='submit' value='Save'>";
    html += "</form>";
    server.send(200, "text/html", html);
}

void handleSave() {
    if (server.hasArg("param")) {
        String param = server.arg("param");
        Serial.println("Parameter saved: " + param);
    }
    server.send(200, "text/html", "<h1>Parameter Saved</h1>");
}
*/