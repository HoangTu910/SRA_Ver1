
const mqtt = require('mqtt');
const DeviceDataService = require('./DeviceDataService');

const brokerUrl = 'mqtt://localhost:1883';
const topic = 'sensors/data';

let client;

const encodedPassword = Buffer.from('123').toString('base64');
console.log(`Encoded Password: ${encodedPassword}`); 

const options = {
    username: 'admin', // Correct username
    password: '123', // Correct password, base64 encoded
};

function initMQTT() {
    client = mqtt.connect(brokerUrl, options);

    client.on('connect', () => {
        console.log('Connected to MQTT broker');
        client.subscribe(topic, { qos: 1 }, (err) => {
            if (err) {
                console.error(`Failed to subscribe to ${topic}:`, err);
            } else {
                console.log(`Subscribed to ${topic}`);
            }
        });
    });

    client.on('message', async (topic, message) => {
        const startTime = Date.now();
        console.log(`Raw ${topic}:`, message.toString());
        try {
            const topicParts = topic.split('/');
            const deviceId = topicParts[1];
            const data = JSON.parse(message.toString());

            console.log('Data: ', data);
            console.log(`Received data on topic ${topic}:`);
            console.log(`Heart Rate: ${data.heart_rate}`);
            console.log(`SpO2: ${data.spO2}`);
            console.log(`Temperature: ${data.temperature}`);
            console.log(`Device ID: ${data.id_device.toString()}`);

            // Write data to Firestore
            await DeviceDataService.createDeviceData(data, deviceId);
            const endTime = Date.now();
            console.log(`Processed message in ${endTime - startTime}ms`);
        } catch (e) {
            console.error('Failed to parse message or write to Firestore:', e);
        }
    });

    client.on('error', (err) => {
        console.error('Connection error:', err);
    });

    client.on('reconnect', () => {
        console.log('Reconnecting to MQTT broker...');
    });

    client.on('offline', () => {
        console.log('MQTT client is offline');
    });

    client.on('close', () => {
        console.log('MQTT connection closed');
    });
}

module.exports = {
    initMQTT
};
