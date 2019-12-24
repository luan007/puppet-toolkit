#include "HID-Project.h"
#define OFFSET_PORT 10
#define OFFSET_MAC 0
#define OFFSET_STRINGS 100
#include <avr/wdt.h>
#include <EEPROM.h>
#include <SerialCommands.h>
#include <ArduinoUniqueID.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
char serial_command_buffer_[100];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");
EthernetClient client;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[50];
int port = 80;
int ethernet_state = 0;

void read_eeprom_string(int addr, char* s)
{
  int add = OFFSET_STRINGS + addr * 50;
  int i;
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != 0xff && k != '\0' && len < 50)  //Read until null character
  {
    k = EEPROM.read(add + len);
    s[len] = k;
    len++;
  }
  s[len] = '\0';
}

void setup() {
  MCUSR = 0; //clear reset
  read_eeprom_string(0, server);
  Serial.begin(115200);
  init_serial_cmds();
  BootKeyboard.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  size_t _mac_is_empty = 1;
  EEPROM.get(OFFSET_PORT, port);
  for (size_t i = 0; i < 6; i++) {
    if (EEPROM.read(i + OFFSET_MAC) != 0 && EEPROM.read(i + OFFSET_MAC) != 0xFF) {
      _mac_is_empty = 0;
      break;
    }
  }
  if (_mac_is_empty) {
    for (size_t i = 0; i < 3; i++)
    {
      mac[i + 3] = UniqueID[i + UniqueIDsize - 3];
    }
  } else {
    for (size_t i = 0; i < 6; i++) {
      mac[i] = EEPROM.read(i + OFFSET_MAC);
    }
  }
}


long next_tick = 0;

void loop_request_json() {
  if (millis() < next_tick) return;
  client.setTimeout(10000);
  if (!client.connect(server, port)) {
    Serial.println(F("Connection failed"));
    next_tick = millis() + 1000;
    return;
  }
  send_request(&client);
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    next_tick = millis() + 1000;
    return;
  }

  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status + 9, "200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    next_tick = millis() + 1000;
    return;
  }

  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    next_tick = millis() + 1000;
    return;
  }

  const size_t capacity = 200;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, client);

  client.stop();
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    next_tick = millis() + 1000;
    return;
  }
  if (doc["state"].as<long>() > 0) {
    BootKeyboard.write(KEY_F15);
  }
  next_tick = millis() + 1000;
}


void loop() {
  //  if (BootKeyboard.getProtocol() == HID_BOOT_PROTOCOL)
  //    digitalWrite(pinLed, HIGH);
  //  else
  //    digitalWrite(pinLed, LOW);
  //

  serial_commands_.ReadSerial();
  if (ethernet_state == 0/* && Ethernet.linkStatus() == LinkON*/) {
    Serial.println("Fetching mac addr, this could take up to 5 seconds..");
//    Serial.println(mac, HEX);
    if (Ethernet.begin(mac, 5000, 1000) == 0) {
      Serial.println("Failed to get dhcp address.. retrying soon, bad mac addr maybe?");
      Serial.println(server);
      return;
    } else {
      ethernet_state = 1;
    }
  }
  else if (ethernet_state == 1) {
    //check broken link
    /*if(Ethernet.linkStatus() == LinkOFF) {
      //bad..
      Serial.println("Link broken, rebooting");
      reboot();
      }  */
    Ethernet.maintain();
  }
  else {
    //unknown!
    return;
  }

  digitalWrite(LED_BUILTIN, 1);
  loop_request_json();
  digitalWrite(LED_BUILTIN, 0);
  // Light led if keyboard uses the boot protocol (normally while in bios)
  // Keep in mind that on a 16u2 and Arduino Micro HIGH and LOW for TX/RX Leds are inverted.
  //  Serial.println(BootKeyboard.getProtocol() == HID_BOOT_PROTOCOL ? 1 : 0);
  //  delay(5500);
  //  BootKeyboard.write(KEY_F15);
}
