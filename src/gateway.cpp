#ifndef _GATEWAY_H
#define _GATEWAT_H
#include "gateway.h"
#include "handshake.h"

// Network credentials
const char* ssid = "The Jade Coffee and Tea";
const char* password = "caphengon";

// MQTT Broker
const int mqtt_port = 1885;
const char* mqtt_user = "admin";
const char* mqtt_pass = "123";
const char* mqtt_server = "113.161.225.11";
const char* device_id = "newdevice123";
const char* dataTopic = "sensors/data";
const char* publicKeyTopic = "encrypt/dhexchange";

bool receivedSynAck = false; 
// WiFi and MQTT client objects
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;

    for (int i = 0; i < length; i++) {
        messageTemp += (char)message[i];
    }
    Serial.println(messageTemp);

    String mes = bytesToHexString(message, length);
    if (String(topic) == "handshake/syn-ack") {
        if (mes == String(SERVER_SYN_ACK, HEX)) { // Convert SERVER_SYN_ACK to hex string
            Serial.println("SYN-ACK match found. Received SYN-ACK.");
            receivedSynAck = true;
        } else {
            Serial.println("SYN-ACK mismatch.");
        }
    } else {
        Serial.println("Topic did not match 'handshake/syn-ack'.");
    }
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
            Serial.println("connected");
            // client.publish("outTopic", "hello world");
            // client.subscribe("inTopic");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

uint8_t mqtt_setup()
{
    if (!client.connected()) {
        Serial.println("MQTT client not connected, attempting to reconnect...");
        reconnect();
    } else {
        client.loop();  
    }
    return 0;
}

void publishFrame(Encrypt_Frame_t &frame, const char* topic, unsigned long long clen)
{
    String json = serializeToJSON(frame, clen);
    
    bool result = client.publish(topic, json.c_str());
    Serial.print("Payload length: ");
    Serial.println(json.length());
    if (result) {
        Serial.println("Published JSON successfully");
    } else {
        Serial.print("Failed to publish JSON. Error code: ");
        Serial.println(result);
    }
}

String bytesToHexString(byte* payload, unsigned int length) {
    String hexString = "";
    for (int i = 0; i < length; i++) {
        if (payload[i] < 0x10) {
            hexString += "0"; // Add leading zero if necessary
        }
        hexString += String(payload[i], HEX);
    }
    return hexString;
}

void publishPublicKey(DH_KEY publicKey, const char *topic)
{
    if (client.publish(publicKeyTopic, (const char *)publicKey, DH_KEY_LENGTH)) {
        Serial.println("DH key published successfully.");
    } else {
        Serial.println("Failed to publish DH key.");
    }
}

void performKeyExchange()
{
    Serial.println("Define");
    DH_KEY clientPublic, clientPrivate, clientSecret;

    Serial.println("Random");
    time_t seed;
	time(&seed);
	srand((unsigned int)seed);

    Serial.println("Generate Key");
    DH_generate_key_pair(clientPublic, clientPrivate);
    Serial.println("Finished generate key");
    publishPublicKey(clientPublic, publicKeyTopic);

    Serial.print("[PUBLIC KEY]: ");
    for (int i = 0; i < DH_KEY_LENGTH; i++) {
        Serial.printf("%02X", clientPublic[i]);  // Print as hex
    }
    Serial.println();
}
#endif
