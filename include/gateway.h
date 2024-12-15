#ifndef GATEWAY_H
#define GATEWAY_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "frameparsing.h"
#include "dhexchange.h"
#include "ecdh.h"

extern const char* ssid;
extern const char* password;

extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* mqtt_user; // MQTT username
extern const char* mqtt_pass; // MQTT password
extern const char* device_id;  
extern bool receivedSynAck;
extern uint8_t serverPublicKey[ECC_PUB_KEY_SIZE];

extern const char* dataTopic;
extern const char* publicKeyTopic;

extern bool isReceivePublicFromServer;

extern WiFiClient espClient;
extern PubSubClient client;

static uint8_t puba[ECC_PUB_KEY_SIZE];
static uint8_t prva[ECC_PRV_KEY_SIZE];
static uint8_t seca[ECC_PUB_KEY_SIZE];
static uint8_t pubb[ECC_PUB_KEY_SIZE];
static uint8_t prvb[ECC_PRV_KEY_SIZE];
static uint8_t secb[ECC_PUB_KEY_SIZE];

// Function declarations
void setup_wifi();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();
uint8_t mqtt_setup();
void publishFrame(Encrypt_Frame_t& frame, const char* topic, unsigned long long clen);
String bytesToHexString(byte* payload, unsigned int length);
bool publishPublicKey(DH_KEY publicKey, const char* topic);
int serverPublicCheck(DH_KEY serverPublicKey);
bool performKeyExchange(unsigned char *key);
void generateSecretKey(DH_KEY clientPrivate, DH_KEY clientSecret);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void generatePublicAndSecret(uint8_t publicKey[ECC_PUB_KEY_SIZE], uint8_t privateKey[ECC_PRV_KEY_SIZE]);
#endif // WIFI_CONFIG_H
