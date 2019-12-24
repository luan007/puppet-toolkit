#include <SerialCommands.h>

void send_request(Stream* client) {
  client->print("GET /?t=");
  client->print(millis());
  client->println(" HTTP/1.1");
  client->print("Host: ");
  client->print(server);
  client->print(":");
  client->println(port);
  client->println("Connection: close");
}

void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}

//called for ON command
void cmd_help(SerialCommands* sender)
{
  //  digitalWrite(kLedPin, HIGH);
  sender->GetSerial()->println("MAN / ");
  sender->GetSerial()->println("print_req \n\t prints demo request");
  sender->GetSerial()->println("get_mac \n\t reads mac address");
  sender->GetSerial()->println("set_mac [0][1][2][3][4][5] \n\t sets mac address to eeprom (6-hex)");
  sender->GetSerial()->println("get_port \n\t reads port number");
  sender->GetSerial()->println("set_port [int] \n\t sets port address to eeprom (1-int)");
  sender->GetSerial()->println("get_param [server] \n\t gets string param");
  sender->GetSerial()->println("set_param [server] [char(100)] \n\t sets string param");
  sender->GetSerial()->println("reset \n\t reboot this board");
}

void reboot() {
  wdt_enable(WDTO_15MS); // turn on the WatchDog and don't stroke it.
  for (;;) {
    // do nothing and wait for the eventual...
  }
}

void cmd_reset(SerialCommands* sender) {
  sender->GetSerial()->println("bye.");
  reboot();
}

void cmd_get_port(SerialCommands* sender) {
  sender->GetSerial()->print("port=\t");
  sender->GetSerial()->println(port);
}

void cmd_set_port(SerialCommands* sender) {
  char* mac_str = sender->Next();
  if (mac_str == NULL) {
    sender->GetSerial()->println("ERROR");
    return;
  }
  int val = atoi(mac_str);
  sender->GetSerial()->print("set\t");
  sender->GetSerial()->println(val);
  EEPROM.put(OFFSET_PORT, val);
}


void cmd_get_mac(SerialCommands* sender) {
  for (size_t i = 0; i < 6; i++) {
    sender->GetSerial()->print(byte(mac[i]), HEX);
  }
  sender->GetSerial()->println();
}

void cmd_set_mac(SerialCommands* sender)
{
  char _ready = 1;
  for (int i = 0; i < 6; i++) {
    char* mac_str = sender->Next();
    if (mac_str == NULL)
    {
      sender->GetSerial()->println("ERROR PARAM_ERR");
      _ready = 0;
      return;
    }
    char b = (char)ascii2byte(mac_str[0], mac_str[1]);
    EEPROM.write(i + OFFSET_MAC, byte(b));
    sender->GetSerial()->print(byte(b), HEX);
  }
  sender->GetSerial()->println("");
  sender->GetSerial()->println("END");
}



const char ar[] = "0123456789ABCDEF";
byte ascii2byte(char a, char b)
{
  byte v = 0;
  for (uint8_t i = 0; i < 16; i++)
  {
    if (ar[i] == a) v += (i * 16);
    if (ar[i] == b) v += i;
  }
  return v;
}

int cmd_helper_gs(char* cmd_sel) {
  if (cmd_sel == NULL) {
    return -1;
  }
  if (strcmp (cmd_sel, "host")) {
    return 0;
  }
  return -1;
}

void cmd_set_eeprom_string(SerialCommands* sender)
{
  int sel = cmd_helper_gs(sender->Next());
  if (sel == -1) {
    sender->GetSerial()->println("WRONG PARAM TYPE");
    return;
  }
  sender->GetSerial()->println("set eeprom");
  char* mac_str = sender->Next();
  sender->GetSerial()->println(mac_str);
  int i = 0;
  while (true) {
    int offset = OFFSET_STRINGS + i + sel * 50;
    sender->GetSerial()->print("LOC\t");
    sender->GetSerial()->print(offset);
    sender->GetSerial()->print("\t");
    sender->GetSerial()->println(mac_str[i], HEX);
    EEPROM.write(offset, mac_str[i]);
    if (mac_str[i] == '\0') break;
    i++;
  }
  sender->GetSerial()->println(mac_str);
}

void cmd_get_eeprom_string(SerialCommands* sender)
{
  int sel = cmd_helper_gs(sender->Next());
  if (sel == -1) {
    sender->GetSerial()->println("WRONG PARAM TYPE");
    return;
  }
  sender->GetSerial()->println("IN RAM STRING = ");
  sender->GetSerial()->println(server);
}

void cmd_print_req(SerialCommands* sender)
{
  sender->GetSerial()->println("--\tHTTP\t--");
  send_request(sender->GetSerial());
}

SerialCommand _cmd_help("help", cmd_help);
SerialCommand _cmd_reset("reset", cmd_reset);
SerialCommand _cmd_get_mac("get_mac", cmd_get_mac);
SerialCommand _cmd_set_mac("set_mac", cmd_set_mac);
SerialCommand _cmd_get_port("get_port", cmd_get_port);
SerialCommand _cmd_set_port("set_port", cmd_set_port);
SerialCommand _cmd_set_eeprom_string("set_param", cmd_set_eeprom_string);
SerialCommand _cmd_get_eeprom_string("get_param", cmd_get_eeprom_string);
SerialCommand _cmd_print_req("print_req", cmd_print_req);

void init_serial_cmds() {
  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&_cmd_set_eeprom_string);
  serial_commands_.AddCommand(&_cmd_get_eeprom_string);
  serial_commands_.AddCommand(&_cmd_print_req);
  serial_commands_.AddCommand(&_cmd_set_mac);
  serial_commands_.AddCommand(&_cmd_get_mac);
  serial_commands_.AddCommand(&_cmd_get_port);
  serial_commands_.AddCommand(&_cmd_set_port);
  serial_commands_.AddCommand(&_cmd_help);
  serial_commands_.AddCommand(&_cmd_reset);
}
