#include <WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "ESP32-Access-Point"; // Access Point SSID
const char *password = "123456789";       // Access Point password

WiFiUDP udp;
const int udpPort = 1234;                 // Port to send UDP packets

void setup() {
    Serial.begin(115200);

    // Set up the Access Point
    WiFi.softAP(ssid, password);
    Serial.println("Access Point started");

    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    udp.begin(udpPort);
}

void loop() {
    // Example message to send
    const char *message = "Hello, clients!";
    
    // Broadcast the UDP packet
    udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
    udp.print(message); // Use udp.print() instead of udp.write()
    udp.endPacket();

    Serial.println("Broadcast message sent");

    delay(2000); // Send every 2 seconds
}
