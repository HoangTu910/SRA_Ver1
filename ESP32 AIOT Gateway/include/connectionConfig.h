#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <WiFi.h>
#include <PubSubClient.h>

extern const char* ssid;
extern const char* password;

extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* mqtt_user; // MQTT username
extern const char* mqtt_pass; // MQTT password
extern const char* device_id;   

extern WiFiClient espClient;
extern PubSubClient client;

// Function declarations
void setup_wifi();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();

#endif // WIFI_CONFIG_H
