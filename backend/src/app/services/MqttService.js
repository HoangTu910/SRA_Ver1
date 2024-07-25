const mqtt = require('mqtt');
const HeartrateService = require('../services/HeartrateService');

const brokerUrl = 'mqtt://broker.hivemq.com';
const topic = 'sensor/hoanghuutu';

let client;

function initMQTT() {
    client = mqtt.connect(brokerUrl);

    client.on('connect', () => {
        console.log('Connected to MQTT broker');
        client.subscribe(topic, (err) => {
            if (err) {
                console.error(`Failed to subscribe to ${topic}:`, err);
            } else {
                console.log(`Subscribed to ${topic}`);
            }
        });
    });

    client.on('message', async (topic, message) => {
        console.log(`Raw ${topic}:`, message.toString());
        try {
            const data = JSON.parse(message.toString());
            console.log('Data: ', data);
            console.log(`Received data on topic ${topic}:`);
            console.log(`Heart Rate: ${data.heart_rate}`);
            console.log(`SpO2: ${data.spO2}`);
            console.log(`Temperature: ${data.temperature}`);

            // Write data to Firestore
            await HeartrateService.createHeartrate(data);
        } catch (e) {
            console.error('Failed to parse message or write to Firestore:', e);
        }
    });

    client.on('error', (err) => {
        console.error('Connection error:', err);
    });
}

module.exports = {
    initMQTT
};
