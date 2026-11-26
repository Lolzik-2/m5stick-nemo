// ===== Settings ===== //
const uint8_t channels[] = {1, 6, 11}; // used Wi-Fi channels (available: 1-14)
const bool wpa2 = true; // WPA2 networks
int spamtype = 1; // 1 = funny, 2 = rickroll, maybe more later

/*
  SSIDs:
  - don't forget the \n at the end of each SSID!
  - max. 32 characters per SSID
  - don't add duplicates! You have to change one character at least
*/
char ssids[]={};
uint8_t mac[6];

const char funnyssids[] PROGMEM = {
  "Mom Use This One\n"
  "Abraham Linksys\n"
  "Benjamin FrankLAN\n"
  "Martin Router King\n"
  "John Wilkes Bluetooth\n"
  "Pretty Fly for a Wi-Fi\n"
  "Bill Wi the Science Fi\n"
  "I Believe Wi Can Fi\n"
  "Tell My Wi-Fi Love Her\n"
  "No More Mister Wi-Fi\n"
  "LAN Solo\n"
  "The LAN Before Time\n"
  "Silence of the LANs\n"
  "House LANister\n"
  "Winternet Is Coming\n"
  "Ping’s Landing\n"
  "The Ping in the North\n"
  "This LAN Is My LAN\n"
  "Get Off My LAN\n"
  "The Promised LAN\n"
  "The LAN Down Under\n"
  "FBI Surveillance Van 4\n"
  "Area 51 Test Site\n"
  "Drive-By Wi-Fi\n"
  "Planet Express\n"
  "Wu Tang LAN\n"
  "Darude LANstorm\n"
  "Never Gonna Give You Up\n"
  "Hide Yo Kids, Hide Yo Wi-Fi\n"
  "Loading…\n"
  "Searching…\n"
  "VIRUS.EXE\n"
  "Virus-Infected Wi-Fi\n"
  "Starbucks Wi-Fi\n"
  "Text ###-#### for Password\n"
  "Yell ____ for Password\n"
  "The Password Is 1234\n"
  "Free Public Wi-Fi\n"
  "No Free Wi-Fi Here\n"
  "Get Your Own Damn Wi-Fi\n"
  "It Hurts When IP\n"
  "Dora the Internet Explorer\n"
  "404 Wi-Fi Unavailable\n"
  "Porque-Fi\n"
  "Titanic Syncing\n"
  "Test Wi-Fi Please Ignore\n"
  "Drop It Like It’s Hotspot\n"
  "Life in the Fast LAN\n"
  "The Creep Next Door\n"
  "Ye Olde Internet\n"
};

const char rickrollssids[] PROGMEM = {
  "01 Never gonna give you up\n"
  "02 Never gonna let you down\n"
  "03 Never gonna run around\n"
  "04 and desert you\n"
  "05 Never gonna make you cry\n"
  "06 Never gonna say goodbye\n"
  "07 Never gonna tell a lie\n"
  "08 and hurt you\n"
};

// run-time variables
char emptySSID[32];
char beaconSSID[33];
char randomName[32];
uint8_t channelIndex = 0;
uint8_t macAddr[6];
uint8_t wifi_channel = 1;
uint32_t currentTime = 0;
uint32_t packetSize = 0;
uint32_t packetCounter = 0;
uint32_t attackTime = 0;
uint32_t packetRateTime = 0;

#include <string.h>
#include <WiFi.h>

extern "C" {
#include "esp_wifi.h"
  esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second);
  esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
}

const char* generateRandomName() {
  const char* charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int len = rand() % 10 + 1;
  char* randomName = (char*)malloc((len + 1) * sizeof(char));
  for (int i = 0; i < len; ++i) {
    randomName[i] = charset[rand() % strlen(charset)];
  }
  randomName[len] = '\0';
  return randomName;
}

char* randomSSID() {
  const char* charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int len = rand() % 22 + 7;
  for (int i = 0; i < len; ++i) {
    randomName[i] = charset[rand() % strlen(charset)];
  }
  randomName[len] = '\0';
  return randomName;
}

// beacon frame definition
uint8_t beaconPacket[109] = {
  /*  0 - 3  */ 0x80, 0x00, 0x00, 0x00, // Type/Subtype: management beacon frame
  /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: broadcast
  /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source
  /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSSID

  // Fixed parameters
  /* 22 - 23 */ 0x00, 0x00, // Fragment & sequence number
  /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
  /* 32 - 33 */ 0x64, 0x00, // Interval: every 100ms
  /* 34 - 35 */ 0x01, 0x04, // Capability info

  // SSID parameters (max 32 bytes)
  /* 36 - 37 */ 0x00, 0x20, // Tag: SSID, Tag length: 32
  /* 38 - 69 */ 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,

  // Supported Rates
  /* 70 - 71 */ 0x01, 0x08,
  /* 72 */ 0x82,
  /* 73 */ 0x84,
  /* 74 */ 0x8b,
  /* 75 */ 0x96,
  /* 76 */ 0x24,
  /* 77 */ 0x30,
  /* 78 */ 0x48,
  /* 79 */ 0x6c,

  // Current Channel
  /* 80 - 81 */ 0x03, 0x01,
  /* 82 */      0x01,

  // RSN information
  /*  83 -  84 */ 0x30, 0x18,
  /*  85 -  86 */ 0x01, 0x00,
  /*  87 -  90 */ 0x00, 0x0f, 0xac, 0x02,
  /*  91 -  92 */ 0x02, 0x00,
  /*  93 - 100 */ 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04,
  /* 101 - 102 */ 0x01, 0x00,
  /* 103 - 106 */ 0x00, 0x0f, 0xac, 0x02,
  /* 107 - 108 */ 0x00, 0x00
};

// goes to next channel
void nextChannel() {
  if (sizeof(channels) > 0) {
    uint8_t ch = channels[channelIndex];
    channelIndex = (channelIndex + 1) % sizeof(channels);

    if (ch != wifi_channel && ch >= 1 && ch <= 14) {
      wifi_channel = ch;
      esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
    }
  }
}

void beaconSpam(const char ESSID[]){
  Serial.printf("WiFi SSID: %s\n", ESSID);
  uint8_t set_channel = channels[channelIndex];
  channelIndex = (channelIndex + 1) % sizeof(channels);
  esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);

  // Randomize locally administered unicast MAC for Source & BSSID
  for (int idx = 0; idx < 6; idx++) {
    uint8_t b = random(256);
    if (idx == 0) {
      b = (b & 0xFC) | 0x02;
    }
    beaconPacket[10 + idx] = b;
    beaconPacket[16 + idx] = b;
  }

  int realLen = strlen(ESSID);
  if (realLen > 32) realLen = 32;
  if (realLen < 1) return;

  beaconPacket[34] = wpa2 ? 0x31 : 0x01;
  beaconPacket[37] = realLen;

  for (int i = 0; i < 32; i++) {
    beaconPacket[38 + i] = (i < realLen) ? (uint8_t)ESSID[i] : 0x20;
  }

  beaconPacket[82] = set_channel;
  int txLen = wpa2 ? sizeof(beaconPacket) : 83;

  esp_wifi_80211_tx(WIFI_IF_AP, beaconPacket, txLen, false);
  delay(1);
}

void beaconSpamList(const char list[]){
  int i = 0;
  int j = 0;
  char tmp;
  int ssidsLen = strlen_P(list);
  while (i < ssidsLen) {
    j = 0;
    do {
      tmp = pgm_read_byte(list + i + j);
      j++;
    } while (tmp != '\n' && tmp != '\0' && j <= 32 && i + j < ssidsLen);

    uint8_t ssidLen = (tmp == '\n') ? (j - 1) : j;
    if (ssidLen > 32) ssidLen = 32;

    memset(beaconSSID, 0, sizeof(beaconSSID));
    if (ssidLen > 0) {
      memcpy_P(beaconSSID, list + i, ssidLen);
      beaconSSID[ssidLen] = '\0';
      beaconSpam(beaconSSID);
    }

    i += j;
  }
}
