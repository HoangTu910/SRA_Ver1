const mqtt = require('mqtt');
const { execFile } = require('child_process');
const path = require('path');
const DeviceDataService = require('./DeviceDataService');

const brokerUrl = 'mqtt://localhost:1885';
const topic = 'sensors/data';

const TOPIC_HANDSHAKE_SYN = 'handshake/syn';
const TOPIC_HANDSHAKE_SYN_ACK = 'handshake/syn-ack';
const TOPIC_HANDSHAKE_ACK = 'handshake/ack';
const DATA_TOPIC = 'sensors/data';
const TOPIC_TO_RECEIVE_PUBLIC_FROM_CLIENT = 'encrypt/dhexchange';
const TOPIC_TO_PUBLIC_KEY_TO_CLIENT = 'encrypt/dhexchange-server';

const SYN = Buffer.from([0xA1]); ;
const SYN_ACK = Buffer.from([0xA2]); ;
const ACK = Buffer.from([0xA3]); 

let serverPublicKey = null;
let serverPrivateKey = null;
let serverSecret = null;
let serverReceivePublic = null;

const publicExecutablePath = path.resolve(__dirname, '../diffie-hellman/exec-public');
const secretExecutablePath = path.resolve(__dirname, '../diffie-hellman/exec-secret');

let client;
let handshakeState = {
    initiated: false,
    completed: false,
};

// Function to generate public and private keys
async function generatePublicPrivateKeys() {
    return new Promise((resolve, reject) => {
        execFile(publicExecutablePath, (error, stdout, stderr) => {
            if (error) {
                console.error(`Error executing public/private key file: ${error.message}`);
                return reject(error);
            }

            if (stderr) {
                console.error(`stderr: ${stderr}`);
                return reject(new Error(stderr));
            }

            console.log(`Output from public/private key C++ executable: \n${stdout}`);

            const publicKeyMatch = stdout.match(/Public Key:\s([0-9a-f]+)/);
            const privateKeyMatch = stdout.match(/Private Key:\s([0-9a-f]+)/);

            if (publicKeyMatch && privateKeyMatch) {
                const publicKey = publicKeyMatch[1];
                const privateKey = privateKeyMatch[1];

                console.log("Generated Public Key (Hex):", publicKey);
                console.log("Generated Private Key (Hex):", privateKey);

                resolve({ publicKey, privateKey });
            } else {
                reject(new Error("Could not find public/private keys in the output."));
            }
        });
    });
}

// Function to generate the secret key
async function generateSecretKey(myPrivateHex, anotherPublicHex) {
    return new Promise((resolve, reject) => {
        execFile(secretExecutablePath, [myPrivateHex, anotherPublicHex], (error, stdout, stderr) => {
            if (error) {
                console.error(`Error executing secret key file: ${error.message}`);
                return reject(error);
            }

            if (stderr) {
                console.error(`stderr: ${stderr}`);
                return reject(new Error(stderr));
            }

            console.log(`Output from secret key C++ executable: \n${stdout}`);

            const secretKeyMatch = stdout.match(/Secret Key:\s([0-9a-f]+)/);
            if (secretKeyMatch) {
                const secretKey = secretKeyMatch[1];
                serverSecret = secretKey;
                console.log("Generated Secret Key (Hex):", secretKey);
                resolve(secretKey);
            } else {
                reject(new Error("Could not find the secret key in the output."));
            }
        });
    });
}

(async () => {
    try {
        const { publicKey, privateKey } = await generatePublicPrivateKeys();
        serverPublicKey = publicKey;
        serverPrivateKey = privateKey;
        console.log("Server Public Key (Hex):", serverPublicKey);
        console.log("Server Private Key (Hex):", serverPrivateKey);

    } catch (error) {
        console.error("Error during key generation:", error.message);
    }
})();


function reconstructDecryptedData(decryptedtext) {
    if (!decryptedtext) {
        console.log("Error: Input is null or undefined.");
        return { error: true }; // Return an error object if the input is null or undefined
    }

    //console.log("Decrypted text received: ", decryptedtext);

    let deviceId = "";
    for (let i = 0; i < 32; i++) {
        if (decryptedtext[i] === 0) break;
        deviceId += String.fromCharCode(decryptedtext[i]);
    }
    //console.log("Extracted Device ID: ", deviceId);

    let deviceLen = decryptedtext[32];
    //console.log("Extracted Device Length: ", deviceLen);

    let dataLen = decryptedtext[33];
    //console.log("Extracted Data Length: ", dataLen);

    let result = {
        deviceId: deviceId,
        deviceLen: deviceLen,
        dataLen: dataLen
    };

    if (dataLen >= 5) {
        let heartRate = decryptedtext[35];
        let spO2 = decryptedtext[36];
        let temperature = decryptedtext[37];
        let acceleration = decryptedtext[38];
        let isanomaly = decryptedtext[39];
        result.heartRate = heartRate;
        result.spO2 = spO2;
        result.temperature = temperature;
        result.acceleration = acceleration;
        result.isanomaly = isanomaly
        // console.log("Extracted Heart Rate: ", heartRate);
        // console.log("Extracted SpO2: ", spO2);
        // console.log("Extracted Temperature: ", temperature);
    } else {
        result.error = "Insufficient data length to extract health metrics.";
        console.log("Error: Insufficient data length to extract health metrics.");
    }

    console.log("Final result: ", result);
    return result;
}


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
            console.log('Standard Output:', stdout); 
            const lines = stdout.trim().split('\n');
            const decryptionCompletedCheckLine = lines[0];
            const decryptedDataLine = lines[1];

            if (decryptionCompletedCheckLine.includes("Decryption completed check:")) {
                const decryptedHex = decryptedDataLine.trim().split(' ').map(h => parseInt(h, 16));
                const decryptedBuffer = Buffer.from(decryptedHex);
                resolve(decryptedBuffer);
            } else {
                reject(new Error('Decryption failed'));
            } 
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
        client.subscribe(TOPIC_TO_RECEIVE_PUBLIC_FROM_CLIENT, { qos: 1 }, (err) => {
            if (err) {
                console.error(`Failed to subscribe to ${TOPIC_TO_RECEIVE_PUBLIC_FROM_CLIENT}:`, err);
            } else {
                console.log(`Subscribed to ${TOPIC_TO_RECEIVE_PUBLIC_FROM_CLIENT}`);
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
                const data = JSON.parse(message.toString());
                //console.log('Data: ', data);
                const encryptDataBuffer = Buffer.from(data.dataEncrypted);
                const nonce = Buffer.from(data.nonce);
                // const key = new Uint8Array([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F]);
                serverSecret = generateSecretKey(serverPrivateKey, serverReceivePublic);
                const decryptedData = await decryptData(encryptDataBuffer, nonce, serverSecret);
                // console.log('Decrypted Data Length: ', decryptedData.length);
                // console.log('Decrypted Data Buffer: ', decryptedData);
                console.log('Decrypted Data (Hex): ', decryptedData.toString('hex'));
                const result = reconstructDecryptedData(decryptedData);
                // console.log('Device ID: ', result.deviceId);
                // console.log('Heart Rate: ', result.heartRate);
                // console.log('Temperature: ', result.temperature);
                // console.log('SPO2: ', result.spO2);
                // console.log('Anomaly: ', result.isanomaly);
                // Write data to Firestore
                const uploadData = {
                    heart_rate: result.heartRate,
                    temperature: result.temperature,
                    spO2: result.spO2,
                    acceleration: result.acceleration,
                    isanomaly: result.isanomaly
                };
                
                await DeviceDataService.createDeviceData(uploadData, result.deviceId);
                const endTime = Date.now();
                console.log(`Processed message in ${endTime - startTime}ms`);
            } catch (e) {
                console.error('Failed to parse message or write to Firestore:', e);
            }
        } else if(topic === TOPIC_TO_RECEIVE_PUBLIC_FROM_CLIENT){
            const data = message;
            const publicKeyReceiveFromClient = Buffer.from(data);
            serverReceivePublic = publicKeyReceiveFromClient
            console.log('Public key received: ', serverReceivePublic);
            if(serverReceivePublic != null){
                client.publish(TOPIC_TO_PUBLIC_KEY_TO_CLIENT, serverPublicKey, { qos: 1 }, (err) => {
                    if (err) {
                        console.error('Failed to publish SERVER PUBLIC:', err);
                    } else {
                        console.log('Published SERVER PUBLIC');
                    }
                });
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
