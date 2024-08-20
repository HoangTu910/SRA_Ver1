#ifndef GATEWAY_H
#define GATEWAY_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "frameparsing.h"

extern const char* ssid;
extern const char* password;

extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* mqtt_user; // MQTT username
extern const char* mqtt_pass; // MQTT password
extern const char* device_id;  
extern bool receivedSynAck;

extern const char* dataTopic;

extern WiFiClient espClient;
extern PubSubClient client;

// Function declarations
void setup_wifi();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();
uint8_t mqtt_setup();
void publishFrame(Encrypt_Frame_t& frame, const char* topic, unsigned long long clen);
String bytesToHexString(byte* payload, unsigned int length);
#endif // WIFI_CONFIG_H
