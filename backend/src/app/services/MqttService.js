const mqtt = require('mqtt');
const { execFile } = require('child_process');
const path = require('path');
const DeviceDataService = require('./DeviceDataService');

const brokerUrl = 'mqtt://localhost:1883';
const topic = 'sensors/data';

const TOPIC_HANDSHAKE_SYN = 'handshake/syn';
const TOPIC_HANDSHAKE_SYN_ACK = 'handshake/syn-ack';
const TOPIC_HANDSHAKE_ACK = 'handshake/ack';
const DATA_TOPIC = 'sensors/data';

const SYN = Buffer.from([0xA1]); ;
const SYN_ACK = Buffer.from([0xA2]); ;
const ACK = Buffer.from([0xA3]); ;

let client;
let handshakeState = {
    initiated: false,
    completed: false,
};

const decryptData = (encryptedData, nonce, key) => {
    return new Promise((resolve, reject) => {
        const binaryPath = path.join(__dirname, '../cryptography/src/decrypt_binary');
        
        // Convert inputs to hex strings
        const encryptedHex = Buffer.from(encryptedData).toString('hex');
        const nonceHex = Buffer.from(nonce).toString('hex');
        const keyHex = Buffer.from(key).toString('hex');
        
        const args = [encryptedHex, nonceHex, keyHex];

        execFile(binaryPath, args, { env: { LD_LIBRARY_PATH: path.join(__dirname, '../cryptography/src') } }, (error, stdout, stderr) => {
            if (error) {
                console.error('Error executing binary:', error);
                console.error('Standard Error Output:', stderr);
                return reject(error);
            }
            console.log('Standard Output:', stdout); // Log the output
            const lines = stdout.split('\n');
            const hexDataLine = lines[1]; // Assuming the hex data is in the second line
            const hexValues = hexDataLine.trim().split(' ').filter(v => v.length > 0); // Split and filter empty strings
            const hexString = hexValues.join(''); // Join them into a single string
            resolve(Buffer.from(hexString, 'hex')); 
        });
    });
};


const encodedPassword = Buffer.from('123').toString('base64');
console.log(`Encoded Password: ${encodedPassword}`); 

const options = {
    username: 'admin', // Correct username
    password: '123', // Correct password, base64 encoded
};

function initiateHandshake() {
    // Publish SYN message to start the handshake
    client.publish('handshake/initiate', SYN);
    handshakeState.initiated = true;
    console.log('Handshake initiated.');
}

async function handleHandshakeResponse(message) {
    if (message.toString() === 'SYN-ACK') {
        // Publish ACK message to complete the handshake
        client.publish('handshake/acknowledge', ACK);
        handshakeState.completed = true;
        console.log('Handshake completed.');
    }
}

function initMQTT() {
    client = mqtt.connect(brokerUrl, options);

    client.on('connect', () => {
        console.log('Connected to MQTT broker');
        // Subscribe to topics
        client.subscribe(TOPIC_HANDSHAKE_SYN, { qos: 1 }, (err) => {
            if (err) {
                console.error(`Failed to subscribe to ${TOPIC_HANDSHAKE_SYN}:`, err);
            } else {
                console.log(`Subscribed to ${TOPIC_HANDSHAKE_SYN}`);
            }
        });
        client.subscribe(TOPIC_HANDSHAKE_SYN_ACK, { qos: 1 }, (err) => {
            if (err) {
                console.error(`Failed to subscribe to ${TOPIC_HANDSHAKE_SYN_ACK}:`, err);
            } else {
                console.log(`Subscribed to ${TOPIC_HANDSHAKE_SYN_ACK}`);
            }
        });
        client.subscribe(TOPIC_HANDSHAKE_ACK, { qos: 1 }, (err) => {
            if (err) {
                console.error(`Failed to subscribe to ${TOPIC_HANDSHAKE_ACK}:`, err);
            } else {
                console.log(`Subscribed to ${TOPIC_HANDSHAKE_ACK}`);
            }
        });
        client.subscribe(DATA_TOPIC, { qos: 1 }, (err) => {
            if (err) {
                console.error(`Failed to subscribe to ${DATA_TOPIC}:`, err);
            } else {
                console.log(`Subscribed to ${DATA_TOPIC}`);
            }
        });
    });
    

    client.on('message', async (topic, message) => {
        const startTime = Date.now();
        const hexData = message.toString('hex'); // Convert message to HEX string
        const messageBuffer = Buffer.from(hexData, 'hex'); 
        //console.log(`Raw ${topic}:`, messageBuffer);
        
        if (topic === 'handshake/syn') {
            // Handle SYN from ESP32
            console.log('Handshake SYN received.');
            const commandSyn = messageBuffer[0];
            console.log(`Command SYN value: ${commandSyn.toString(16).toUpperCase()}`);
            client.publish('handshake/syn-ack', SYN_ACK, { qos: 1 }, (err) => {
                if (err) {
                    console.error('Failed to publish SYN-ACK:', err);
                } else {
                    console.log('Published SYN-ACK');
                }
            });
        } else if (topic === 'handshake/syn-ack') {
            // Handle SYN-ACK response
            console.log('Handshake SYN-ACK received.');
            // Wait for ACK from the ESP32
        } else if (topic === 'handshake/ack') {
            if (message.toString() === ACK.toString()) {
                console.log('Handshake ACK received.');
                // client.subscribe('sensors/data', { qos: 1 }, (err) => {
                //     if (err) {
                //         console.error(`Failed to subscribe to sensors/data:`, err);
                //     } else {
                //         console.log('Subscribed to sensors/data');
                //     }
                // });
            }
        } else if (topic === 'sensors/data') {
            try {
                // const topicParts = topic.split('/');
                // const deviceId = topicParts[1];
                const data = JSON.parse(message.toString());
                // console.log('Data: ', data);
                // console.log(`Received data on topic ${topic}:`);
                // console.log(`Heart Rate: ${data.heart_rate}`);
                // console.log(`SpO2: ${data.spO2}`);
                // console.log(`Temperature: ${data.temperature}`);
                // console.log(`Device ID: ${data.id_device.toString()}`);
                console.log('Data: ', data);
                const encryptDataBuffer = Buffer.from(data.dataEncrypted);
                const nonce = Buffer.from(data.nonce);
                const key = new Uint8Array([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F]);

                const decryptedData = await decryptData(encryptDataBuffer, nonce, key);
                console.log('Decrypted Data Length: ', decryptedData.length);
                console.log('Decrypted Data Buffer: ', decryptedData);
                console.log('Decrypted Data (Hex): ', decryptedData.toString('hex'));
                
                // Write data to Firestore
                //await DeviceDataService.createDeviceData(data, deviceId);
                const endTime = Date.now();
                console.log(`Processed message in ${endTime - startTime}ms`);
            } catch (e) {
                console.error('Failed to parse message or write to Firestore:', e);
            }
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
