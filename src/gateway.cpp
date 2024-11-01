#ifndef _GATEWAY_H
#define _GATEWAT_H
#include "gateway.h"
#include "handshake.h"

// Network credentials
// const char* ssid = "FETEL@DESLAB_SV";
// const char* password = "deslabSV";

const char* ssid = "Hoang Tuan";
const char* password = "03081973";

// MQTT Broker
const int mqtt_port = 1885;
const char* mqtt_user = "admin";
const char* mqtt_pass = "123";
const char* mqtt_server = "113.161.225.11";
const char* device_id = "newdevice123";
const char* dataTopic = "sensors/data";
const char* publicKeyTopic = "encrypt/dhexchange";

bool receivedSynAck = false; 
bool isReceivePublicFromServer = false;
// DH_KEY serverPublicKey;
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
    // Serial.print("Message arrived on topic: ");
    // Serial.print(topic);
    // Serial.print(". Message: ");
    // String messageTemp;

    // for (int i = 0; i < length; i++) {
    //     messageTemp += (char)message[i];
    // }
    // Serial.println(messageTemp);

    // String mes = bytesToHexString(message, length);
    // if (strcmp(topic, "handshake/syn-ack") == 0) { 
    //     if (mes == String(SERVER_SYN_ACK, HEX)) { 
    //         Serial.println("SYN-ACK match found. Received SYN-ACK.");
    //         receivedSynAck = true;
    //     } else {
    //         Serial.println("SYN-ACK mismatch.");
    //     }
    // } 
    // else if (strcmp(topic, "encrypt/dhexchange-server") == 0) { 
    //     for (int i = 0; i < DH_KEY_LENGTH; i++) {
    //         serverPublicKey[i] = message[i];  
    //     }
    //     Serial.print("Received Server Public Key: ");
    //     for (int i = 0; i < DH_KEY_LENGTH; i++) {
    //         Serial.printf("%02X", serverPublicKey[i]);
    //     }
    //     Serial.println();
    // }
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
            hexString += "0"; 
        }
        hexString += String(payload[i], HEX);
    }
    return hexString;
}

bool publishPublicKey(DH_KEY publicKey, const char *topic) {
    char hexPublicKey[2 * DH_KEY_LENGTH + 1];
    for (int i = 0; i < DH_KEY_LENGTH; i++) {
        sprintf(hexPublicKey + 2 * i, "%02x", publicKey[i]);
    }
    hexPublicKey[2 * DH_KEY_LENGTH] = '\0'; 
    
    if (client.publish(publicKeyTopic, hexPublicKey)) {
        Serial.println("DH key published successfully as hex.");
        return 1;
    } else {
        Serial.println("Failed to publish DH key.");
        return -1;
    }
}

bool sendPublicToServer(DH_KEY clientPublic, DH_KEY clientPrivate){
    bool state = publishPublicKey(clientPublic, publicKeyTopic);
    if(!state){
        Serial.println("Error: Cannot publish key");
        return 0;
    }
    Serial.print("[PUBLIC KEY]: ");
    for (int i = 0; i < DH_KEY_LENGTH; i++) {
        Serial.printf("%02X", clientPublic[i]);  // Print as hex
    }
    Serial.println();
    return 1;
}

// bool receivePublicFromServer(){
//     if (client.subscribe("encrypt/dhexchange-server")) {
//         Serial.println("Subscribed to public key exchange topic.");
//     } else {
//         Serial.println("Error: Failed to subscribe to server.");
//         return 0;
//     }
//     if(!isReceivePublicFromServer){
//         Serial.println("Error: Cannot receive public from server.");
//         return 0;
//     }
//     isReceivePublicFromServer = false;
//     Serial.print("[RECEIVE KEY FROM SERVER]: ");
//     for (int i = 0; i < DH_KEY_LENGTH; i++) {
//         Serial.printf("%02X", serverPublicKey[i]); 
//     }
//     Serial.println();
//     return 1;
// }

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String mesHex = bytesToHexString(payload, length);
    // Serial.print("Message in HEX: ");
    // Serial.println(mesHex);
    if (strcmp(topic, "encrypt/dhexchange-server") == 0) { 
        for (int i = 0; i < DH_KEY_LENGTH; i++) {
            String byteString = mesHex.substring(i * 4, (i * 4) + 4);
            String firstByte = byteString.substring(0, 2);
            String secondByte = byteString.substring(2, 4);
            int first_byte = strtol(firstByte.c_str(), nullptr, 16); 
            // Serial.print("First: ");
            // Serial.println(first_byte);
            int second_byte = strtol(secondByte.c_str(), nullptr, 16);
            // Serial.print("Second: ");
            // Serial.println(second_byte);
            int firstValue = (char(first_byte) >= '0' && char(first_byte) <= '9') ? (char(first_byte) - '0') : (char(first_byte) - 'a' + 10);
            int secondValue = (char(second_byte) >= '0' && char(second_byte) <= '9') ? (char(second_byte) - '0') : (char(second_byte) - 'a' + 10);
            serverPublicKey[i] = (firstValue << 4) | secondValue; 
        }
        Serial.print("Received Server Public Key: ");
        for (int i = 0; i < DH_KEY_LENGTH; i++) {
            Serial.printf("%02X", serverPublicKey[i]);
        }
        Serial.println();
        isReceivePublicFromServer = true;
    }
    else {
        Serial.print("Topic did not match ");
        Serial.println(String(topic));
    }
}

bool receivePublicFromServer(){
    // Subscribe to the public key exchange topic
    if (client.subscribe("encrypt/dhexchange-server")) {
        Serial.println("Subscribed to public key exchange topic.");
    } else {
        Serial.println("Error: Failed to subscribe to server.");
        return false;
    }

    // Set callback for handling messages
    client.setCallback(mqttCallback);

    // Wait for the message with a timeout
    unsigned long startTime = millis();
    const unsigned long timeout = 5000; // 5 seconds timeout

    while (!isReceivePublicFromServer && (millis() - startTime) < timeout) {
        client.loop(); // Ensure client processes incoming messages
        delay(10);     // Small delay to prevent a tight loop
    }

    // Check if we received the message within the timeout period
    if (!isReceivePublicFromServer) {
        Serial.println("Error: Timed out waiting for public key from server.");
        return false;
    }

    // Reset flag for the next message
    isReceivePublicFromServer = false;
    return true;
}



void subscribeToKeyExchangeTopic(){
    if (client.subscribe("encrypt/dhexchange-server")) {
        Serial.println("Subscribed to public key exchange topic.");
    } else {
        Serial.println("Failed to subscribe.");
    }
}

void generateSecretKey(DH_KEY clientPrivate, DH_KEY clientSecret){
    // Serial.print("Key before generate: ");
    // for(int i = 0; i < DH_KEY_LENGTH; i++){
    //     Serial.print(serverPublicKey[i], HEX);
    // }
    DH_generate_key_secret(clientSecret, clientPrivate, serverPublicKey);
    // Serial.println();
}

int serverPublicCheck(DH_KEY serverPublicKey){
    int sum = 0;
    for(int i = 0; i < DH_KEY_LENGTH; i++){
        sum += serverPublicKey[i];
    }
    return sum;
}

bool performKeyExchange(unsigned char *key)
{
    DH_KEY clientPublic, clientPrivate, clientSecret;

    time_t seed;
	time(&seed);
	srand((unsigned int)seed);

    DH_generate_key_pair(clientPublic, clientPrivate);
    // subscribeToKeyExchangeTopic();
    if(!sendPublicToServer(clientPublic, clientPrivate)){
        Serial.println("Error: Cannot send public to server");
        return 0;
    }
    Serial.println("Sended public to server");
    if(!receivePublicFromServer()){
        Serial.println("Error: Cannot receive public to server");
        return 0;
    }
    Serial.println("Received public to server");
    generateSecretKey(clientPrivate, clientSecret);
    for(int i = 0; i < DH_KEY_LENGTH; i++){
        key[i] = clientSecret[i];
    }
    Serial.println("Key exchange completed!");
    return 1;
}
#endif
