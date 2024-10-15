#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <MPR121.h>
#include <MPR121_Datastream.h>
#include <SD.h>
#include <SPI.h>

#define MPR121_ADDR 0x5A
#define MPR121_INT 4

// MPR121 datastream behaviour constants
const bool MPR121_DATASTREAM_ENABLE = false;

const char* ssid = "ESP32-AP";
const char* password = "123456789";

WebServer server(80); // Create a web server on port 80

void handleRoot();
void handleSave();

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);

  // Set up the serial port
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Starting...");

  // Set up the SD card
  if (!SD.begin(15)) {
    Serial.println("error initializing SD card");
    while (1);
  }

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

  // Set up the Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");

  // Start the server
  server.on("/", handleRoot); // Handle the root URL
  server.begin();
  Serial.println("HTTP server started");
  server.on("/save", HTTP_POST, handleSave);
}

void loop() {
    server.handleClient(); // Handle incoming client requests
    MPR121.updateAll();

    for (int i = 0; i < 12; i++) {
      if (MPR121.isNewTouch(i)) {
        Serial.print("electrode ");
        Serial.print(i, DEC);
        Serial.println(" was just touched");
      } else if (MPR121.isNewRelease(i)) {
        Serial.print("electrode ");
        Serial.print(i, DEC);
        Serial.println(" was just released");
      }
    }

    if (MPR121_DATASTREAM_ENABLE) {
      MPR121_Datastream.update();
    }
}

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