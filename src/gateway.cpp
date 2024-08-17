#include "gateway.h"

// Network credentials
const char* ssid = "OWLCLUB";
const char* password = "OwlclubUS";

// MQTT Broker
const int mqtt_port = 1883;
const char* mqtt_user = "admin";
const char* mqtt_pass = "123";
const char* mqtt_server = "113.161.225.11";
const char* device_id = "newdevice123";

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
