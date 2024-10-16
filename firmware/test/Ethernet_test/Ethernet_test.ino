#include <UIPEthernet.h>

// Replace with your network credentials
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 103);  // Static IP for the ESP32
unsigned int localPort = 8888;    // Local port to listen on
IPAddress remoteIp(192, 168, 1, 105); // Replace with your remote device IP
unsigned int remotePort = 8888;   // Remote port to send data to

EthernetUDP Udp;

void setup() {
  Serial.begin(115200);
  // Start Ethernet
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip); // Start with a static IP if DHCP fails
  }
  
  Udp.begin(localPort);
  Serial.print("Local IP: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Prepare the UDP packet
  String message = "Hello, UDP!";
  Udp.beginPacket(remoteIp, remotePort);
  Udp.write(message.c_str(), message.length());
  Udp.endPacket();

  Serial.println("Packet sent");
  delay(1000);  // Wait for 1 second before sending again
}
