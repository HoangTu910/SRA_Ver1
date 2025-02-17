const mqtt = require('mqtt');
const { execFile } = require('child_process');
const path = require('path');
const DeviceDataService = require('./DeviceDataService');

const brokerUrl = 'mqtt://localhost:1885';
const topic = 'sensors/data';

const DATA_TOPIC = 'sensors/data';
const TOPIC_TO_RECEIVE_PUBLIC_FROM_CLIENT = 'encrypt/dhexchange';
const TOPIC_HANDSHAKE_ECDH = 'handshake/ecdh';
const TOPIC_HANDSHAKE_ECDH_SEND = 'handshake-send/ecdh';

let serverPublicKey = null;
let serverPrivateKey = null;
let serverSecretKey = null;
let serverReceivePublic = null;


const publicExecutablePath = path.resolve(__dirname, '../diffie-hellman/exec-ecdh-public');

let client;

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

            const publicKeyMatch = stdout.match(/Public Key:\s([0-9a-f]+)/);
            const privateKeyMatch = stdout.match(/Private Key:\s([0-9a-f]+)/);

            if (publicKeyMatch && privateKeyMatch) {
                const publicKey = publicKeyMatch[1];
                const privateKey = privateKeyMatch[1];

                resolve({ publicKey, privateKey });
            } else {
                reject(new Error("Could not find public/private keys in the output."));
            }
        });
    });
}

async function generateSecretKey(myPrivateHex, anotherPublicHex) {
    return new Promise((resolve, reject) => {
      const secretExecutablePath = '/home/iot-bts2/HHT_AIT/backend/src/app/diffie-hellman/exec-ecdh-secret';
  
      const child = execFile(secretExecutablePath, (error, stdout, stderr) => {
        if (error) {
          return reject(new Error(`Process error: ${error.message}`));
        }
        if (stderr) {
          console.error(`stderr: ${stderr}`);
        }

        const output = stdout.trim();
        const secretKeyMatch = output.match(/[0-9a-fA-F]+/);
        if (secretKeyMatch) {
          const secretKey = secretKeyMatch[0];
          resolve(secretKey);
        } else {
          reject(new Error("Could not find the secret key in the output."));
        }
      });
  
      child.stdin.write(`${myPrivateHex} ${anotherPublicHex}\n`);
      child.stdin.end();
  
      const timeout = setTimeout(() => {
        child.kill();
        reject(new Error("Process timed out"));
      }, 10000);
  
      child.on('close', () => {
        clearTimeout(timeout);
      });
    });
  }


async function initializeKeys() {
    try {
        const { publicKey, privateKey } = await generatePublicPrivateKeys();
        serverPublicKey = publicKey;
        serverPrivateKey = privateKey;
        console.log("-- Generated [Public] Key:", serverPublicKey.toString('hex').slice(0, 16) + "...");
        console.log("-- Generated [Private] Key:", privateKey.toString('hex').slice(0, 16) + "..."); // Never log full private keys
        return true;
    } catch (error) {
        console.error('Key initialization failed:', error);
        return false;
    }
}

function parseSensorData(hexString) {
    let bytes = [];
    for (let i = 0; i < hexString.length; i += 2) {
        bytes.push(parseInt(hexString.substr(i, 2), 16));
    }

    let data = {
        heartRate: bytes[0],     
        spo2: bytes[1],         
        temperature: bytes[2],   
        acceleration: bytes[3]   
    };

    return data;
}


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

function decryptData(ciphertextHex, nonceHex, keyHex) {
    return new Promise((resolve, reject) => {
      const executablePath = '/home/iot-bts2/HHT_AIT/backend/src/app/cryptography/exec-decrypt';
      
      execFile(executablePath, [ciphertextHex, nonceHex, keyHex], (error, stdout, stderr) => {
        if (error) {
          return reject(new Error(`Execution error: ${error.message}`));
        }
        if (stderr) {
          console.error('stderr:', stderr);
        }
        resolve(stdout.trim());
      });
    });
  }


const encodedPassword = Buffer.from('123').toString('base64');
console.log(`Encoded Password: ${encodedPassword}`);

const options = {
    username: 'admin', // Correct username
    password: '123', // Correct password, base64 encoded
};

const TOPICS = {
    HANDSHAKE_SYN: 'handshake/syn',
    HANDSHAKE_SYN_ACK: 'handshake/syn-ack',
    HANDSHAKE_ACK: 'handshake/ack',
    SENSOR_DATA: 'sensors/data',
    CLIENT_PUBLIC_KEY: 'topic/client-public-key', 
    ECDH_HANDSHAKE: 'handshake/ecdh'       
  };
  
const MESSAGE_HANDLERS = {
    [TOPICS.SENSOR_DATA]: handleSensorData,
    [TOPICS.CLIENT_PUBLIC_KEY]: handleClientPublicKey,
    [TOPICS.ECDH_HANDSHAKE]: parseFrame
};

async function handleSensorData(message) {
    // const data = JSON.parse(message.toString());
    // const { dataEncrypted, nonce } = data;

    // const serverSecret = await generateSecretKey(serverPrivateKey, serverReceivePublic);
    // console.log('Server Secret:', serverSecret);

    // const decryptedData = await decryptData(
    //     Buffer.from(dataEncrypted),
    //     Buffer.from(nonce),
    //     serverSecret
    // );

    // console.log('Decrypted Data (Hex):', decryptedData.toString('hex'));
    // const result = reconstructDecryptedData(decryptedData);

    // await uploadToFirestore(result);
}

async function handleClientPublicKey(message) {
    const messageBuffer = parseMessageToBuffer(message);
    serverReceivePublic = messageBuffer.toString();
    console.log('[PUBLIC RECEIVED]:', serverReceivePublic);

    if (serverReceivePublic) {
        await publishWithCallback(
        TOPICS.SERVER_PUBLIC_KEY,
        serverPublicKey.toString('hex'),
        'SERVER PUBLIC'
        );
    }
}

async function handleEcdhHandshake(message, identifierId, packetType) {
    try {
        // State 1: Parse and validate frame
        const frame = parseHandshakeFrame(message, identifierId, packetType);
        logHandshakeFrame(frame);

        // State 2: Store client public key
        serverReceivePublic = frame.publicKey;
        // console.log('Received client public key:', serverReceivePublic.toString('hex'));

        // State 3: Generate server keys
        const keysInitialized = await initializeKeys();
        if (!keysInitialized) {
            throw new Error('Failed to generate server keys');
        }
        
        // State 4: Publish server public key
        const pubKeyBuffer = Buffer.isBuffer(serverPublicKey) ? serverPublicKey : Buffer.from(serverPublicKey, 'hex');
        client.publish(TOPIC_HANDSHAKE_ECDH_SEND, pubKeyBuffer, { qos: 1 }, (err) => {
            if (err) {
                reject(new Error(`Publish failed: ${err.message}`));
            } else {
                console.log('-- Successfully published public key to', TOPIC_HANDSHAKE_ECDH_SEND);
            }
        });

        //State 5: Server compute secret key
        serverReceivePublic = serverReceivePublic.toString('hex');
        serverSecretKey = await generateSecretKey(serverPrivateKey, serverReceivePublic);
        console.log("-- Generated secret key:", serverSecretKey.toString('hex').slice(0, 16) + "...");

    } catch (error) {
        console.error('Handshake error:', error.message);
        // Implement retry logic or error recovery here
    }
}

async function handleDataFrame(message, identifierId, packetType) {
    const ACK_PACKET = Buffer.from([0x02]);

    try {
        // State 1: Parse and validate the frame
        const frame = parseDataFrame(message, identifierId, packetType);
        if (!frame) {
            throw new Error('[DAMN] Invalid data frame received -_-');
        }
        console.log('[1/4] Parse data frame completed')
        logServerDataFrame(frame);

        // State 2: Decrypt the data payload
        const encryptedHex = frame.encryptedPayload.toString('hex');
        const nonceHex = frame.nonce.toString('hex');

        const decryptedData = await decryptData(encryptedHex, nonceHex, serverSecretKey);
        if (!decryptedData) {
            throw new Error('[DAMN] Decryption failed or returned empty data -_-');
        }
        console.log(`[2/4] Decrypt data completed:`, decryptedData);

        // State 3: Extract sensor data
        const data = parseSensorData(decryptedData);
        if (!data) {
            throw new Error('[DAMN] Failed to parse sensor data from decrypted payload -_-');
        }
        console.log(`[3/4] Parsed sensor data completed`);
        console.log(data);

        //State ?: Send data to database (FIX ME)

        // State 4: Send ACK response
        await publishAck(TOPIC_HANDSHAKE_ECDH_SEND, ACK_PACKET);
        console.log('[NICE] Everything is done')

    } catch (error) {
        console.error(`-- Data frame error: ${error.message}`);
        // Optional: Implement retry logic or recovery mechanism
    }
}

/**
 * Helper function to publish ACK response.
 * Returns a Promise to ensure proper handling with async/await.
 */
function publishAck(topic, ackPacket) {
    return new Promise((resolve, reject) => {
        client.publish(topic, ackPacket, { qos: 1 }, (err) => {
            if (err) {
                reject(new Error(`Failed to publish ACK to ${topic}: ${err.message}`));
            } else {
                console.log(`[4/4] Publish ACK to ${topic} completed`);
                resolve();
            }
        });
    });
}

// Helper functions
function parseMessageToBuffer(message) {
    return Buffer.from(message.toString('hex'), 'hex');
}

async function publishWithCallback(topic, message, description) {
    return new Promise((resolve, reject) => {
        client.publish(topic, message, { qos: 1 }, (err) => {
        if (err) {
            console.error(`Failed to publish ${description}:`, err);
            reject(err);
        } else {
            console.log(`Published ${description}`);
            resolve();
        }
        });
    });
}

async function uploadToFirestore(data) {
    const uploadData = {
        heart_rate: data.heartRate,
        temperature: data.temperature,
        spO2: data.spO2,
        acceleration: data.acceleration,
        isanomaly: data.isanomaly
    };
    await DeviceDataService.createDeviceData(uploadData, data.deviceId);
}

function parseFrame(message) {
    if (message.length < 7) {
        throw new Error("Message too short to parse header");
    }

    const preamble = message.readUInt16LE(0);
    const identifierId = message.readUInt32LE(2);
    const packetType = message.readUInt8(6);

    // Validate preamble 
    if (preamble !== 0xAA55) {
        throw new Error("Invalid preamble");
    }

    // Dispatch based on packet type
    switch (packetType) {
        case 0x03: // Handshake Frame
            return handleEcdhHandshake(message, identifierId, packetType);
        case 0x01: // Data Frame
            return handleDataFrame(message, identifierId, packetType);
        default:
            throw new Error(`Unknown packet type: 0x${packetType.toString(16).padStart(2, '0')}`);
    }
}

function parseHandshakeFrame(message, identifierId, packetType) {
    const publicKeyStart = 9;
    const publicKeyLength = 72;   
    const authTagLength = 16;    

    return {
        preamble: message.readUInt16LE(0),
        identifierId: identifierId,
        packetType: packetType,
        sequenceNumber: message.readUInt16LE(7),
        publicKey: message.subarray(publicKeyStart, publicKeyStart + publicKeyLength),
        authTag: message.subarray(publicKeyStart + publicKeyLength, publicKeyStart + publicKeyLength + authTagLength)
    };
}


function parseDataFrame(message, expectedIdentifierId, expectedPacketType) {
    const NONCE_SIZE = 16;          
    const AUTH_TAG_SIZE = 16;

    // Parse fixed header fields
    const s_preamble = message.readUInt16LE(0);           // offset 0, 2 bytes
    const s_identifierId = message.readUInt32LE(2);         // offset 2, 4 bytes
    const s_packetType = message.readUInt8(6);              // offset 6, 1 byte
    const s_sequenceNumber = message.readUInt16LE(7);       // offset 7, 2 bytes
    const s_timestamp = message.readBigUInt64LE(9);         // offset 9, 8 bytes
    const s_nonce = message.subarray(17, 17 + NONCE_SIZE);   // offset 17, 16 bytes

    const s_payloadLength = message.readUInt16LE(33);       // offset 33, 2 bytes

    const encryptedPayloadStart = 35;                       // immediately after header
    const encryptedPayloadEnd = encryptedPayloadStart + s_payloadLength;
    const authTagStart = encryptedPayloadEnd;
    const authTagEnd = authTagStart + AUTH_TAG_SIZE;

    const s_encryptedPayload = message.subarray(encryptedPayloadStart, encryptedPayloadEnd);
    const s_authTag = message.subarray(authTagStart, authTagEnd);

    if (s_identifierId !== expectedIdentifierId) {
        throw new Error(`Identifier ID mismatch: expected ${expectedIdentifierId}, got ${s_identifierId}`);
    }
    if (s_packetType !== expectedPacketType) {
        throw new Error(`Packet type mismatch: expected ${expectedPacketType}, got ${s_packetType}`);
    }

    return {
        preamble: s_preamble,
        identifierId: s_identifierId,
        packetType: s_packetType,
        sequenceNumber: s_sequenceNumber,
        timestamp: s_timestamp,
        nonce: s_nonce,
        payloadLength: s_payloadLength,
        encryptedPayload: s_encryptedPayload,
        authTag: s_authTag
    };
}

function logHandshakeFrame(frame) {
    let pubKeyHex = frame.publicKey.toString('hex');

    if (pubKeyHex.length > 32) {
        pubKeyHex = pubKeyHex.substring(0, 32) + '...';
    }

    console.log("-- Parsed Handshake Frame:");
    console.table([
        { "Field": "Preamble",       "Value": `0x${frame.preamble.toString(16)}` },
        { "Field": "Identifier ID",    "Value": `0x${frame.identifierId.toString(16)}` },
        { "Field": "Packet Type",      "Value": `0x${frame.packetType.toString(16)}` },
        { "Field": "Sequence Number",  "Value": frame.sequenceNumber },
        { "Field": "Public Key",       "Value": pubKeyHex },
        { "Field": "Auth Tag",         "Value": frame.authTag.toString('hex') }
    ]);
}


function logServerDataFrame(frame) {
    console.log("-- Parsed Server Data Frame:");
    console.table([
        { "Field": "Preamble", "Value": `0x${frame.preamble.toString(16)}` },
        { "Field": "Identifier ID", "Value": `0x${frame.identifierId.toString(16)}` },
        { "Field": "Packet Type", "Value": `0x${frame.packetType.toString(16)}` },
        { "Field": "Sequence Number", "Value": frame.sequenceNumber },
        { "Field": "Timestamp", "Value": frame.timestamp.toString() },
        { "Field": "Nonce", "Value": frame.nonce.toString('hex') },
        { "Field": "Payload Length", "Value": frame.payloadLength },
        { "Field": "Encrypted Payload", "Value": frame.encryptedPayload.toString('hex') },
        { "Field": "Auth Tag", "Value": frame.authTag.toString('hex') }
    ]);
}


function initMQTT() {
    client = mqtt.connect(brokerUrl, options);

    client.on('connect', () => {
        console.log('Connected to MQTT broker');
        client.subscribe(DATA_TOPIC, { qos: 1 }, (err) => {
            if (err) {
                console.error(`Failed to subscribe to ${DATA_TOPIC}:`, err);
            } else {
                console.log(`-- Subscribed to [${DATA_TOPIC}]`);
            }
        });
        client.subscribe(TOPIC_TO_RECEIVE_PUBLIC_FROM_CLIENT, { qos: 1 }, (err) => {
            if (err) {
                console.error(`Failed to subscribe to ${TOPIC_TO_RECEIVE_PUBLIC_FROM_CLIENT}:`, err);
            } else {
                console.log(`-- Subscribed to [${TOPIC_TO_RECEIVE_PUBLIC_FROM_CLIENT}]`);
            }
        });
        client.subscribe(TOPIC_HANDSHAKE_ECDH, { qos: 1 }, (err) => {
            if (err) {
                console.error(`Failed to subscribe to ${TOPIC_HANDSHAKE_ECDH}:`, err);
            } else {
                console.log(`-- Subscribed to [${TOPIC_HANDSHAKE_ECDH}]`);
            }
        });
    });

    client.on('message', async (topic, message) => {
        const handler = MESSAGE_HANDLERS[topic];
        if (handler) {
          try {
            const startTime = Date.now();
            await handler(message);
            const endTime = Date.now();
            console.log(`-> Processed in ${endTime - startTime}ms`);
          } catch (error) {
            console.error(`Error handling ${topic}:`, error);
          }
        } else {
          console.warn(`No handler for topic: ${topic}`);
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



// client.on('message', async (topic, message) => {
//     const startTime = Date.now();
//     const hexData = message.toString('hex'); // Convert message to HEX string
//     const messageBuffer = Buffer.from(hexData, 'hex');
//     //console.log(`Raw ${topic}:`, messageBuffer);

//     if (topic === 'handshake/syn') {
//         // Handle SYN from ESP32
//         console.log('Handshake SYN received.');
//         const commandSyn = messageBuffer[0];
//         console.log(`Command SYN value: ${commandSyn.toString(16).toUpperCase()}`);
//         client.publish('handshake/syn-ack', SYN_ACK, { qos: 1 }, (err) => {
//             if (err) {
//                 console.error('Failed to publish SYN-ACK:', err);
//             } else {
//                 console.log('Published SYN-ACK');
//             }
//         });
//     } else if (topic === 'handshake/syn-ack') {
//         // Handle SYN-ACK response
//         console.log('Handshake SYN-ACK received.');
//         // Wait for ACK from the ESP32
//     } else if (topic === 'handshake/ack') {
//         if (message.toString() === ACK.toString()) {
//             console.log('Handshake ACK received.');
//             // client.subscribe('sensors/data', { qos: 1 }, (err) => {
//             //     if (err) {
//             //         console.error(`Failed to subscribe to sensors/data:`, err);
//             //     } else {
//             //         console.log('Subscribed to sensors/data');
//             //     }
//             // });
//         }
//     } else if (topic === 'sensors/data') {
//         try {
//             const data = JSON.parse(message.toString());
//             //console.log('Data: ', data);
//             const encryptDataBuffer = Buffer.from(data.dataEncrypted);
//             const nonce = Buffer.from(data.nonce);
//             // const key = new Uint8Array([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F]);
//             serverSecret = await generateSecretKey(serverPrivateKey, serverReceivePublic);
//             console.log('Server Secret: ', serverSecret);
//             const decryptedData = await decryptData(encryptDataBuffer, nonce, serverSecret);
//             // console.log('Decrypted Data Length: ', decryptedData.length);
//             // console.log('Decrypted Data Buffer: ', decryptedData);
//             console.log('Decrypted Data (Hex): ', decryptedData.toString('hex'));
//             const result = reconstructDecryptedData(decryptedData);
//             // console.log('Device ID: ', result.deviceId);
//             // console.log('Heart Rate: ', result.heartRate);
//             // console.log('Temperature: ', result.temperature);
//             // console.log('SPO2: ', result.spO2);
//             // console.log('Anomaly: ', result.isanomaly);
//             // Write data to Firestore
//             const uploadData = {
//                 heart_rate: result.heartRate,
//                 temperature: result.temperature,
//                 spO2: result.spO2,
//                 acceleration: result.acceleration,
//                 isanomaly: result.isanomaly
//             };

//             await DeviceDataService.createDeviceData(uploadData, result.deviceId);
//             const endTime = Date.now();
//             console.log(`Processed message in ${endTime - startTime}ms`);
//         } catch (e) {
//             console.error('Failed to parse message or write to Firestore:', e);
//         }
//     } else if (topic === TOPIC_TO_RECEIVE_PUBLIC_FROM_CLIENT) {
//         const hexData = message.toString('hex');
//         const messageBuffer = Buffer.from(hexData, 'hex');
//         const publicKeyReceiveFromClient = messageBuffer;
//         serverReceivePublic = publicKeyReceiveFromClient.toString();
//         console.log('[PUBLIC RECEIVED]: ', serverReceivePublic.toString());
//         if (serverReceivePublic != null) {
//             client.publish(TOPIC_TO_PUBLIC_KEY_TO_CLIENT, serverPublicKey.toString('hex'), { qos: 1 }, (err) => {
//                 if (err) {
//                     console.error('Failed to publish SERVER PUBLIC:', err);
//                 } else {
//                     console.log('Published SERVER PUBLIC');
//                 }
//             });
//             //Wait for client receive and send back noti that received
//         }
//     } else if (topic == TOPIC_HANDSHAKE_ECDH){
//         const s_preamble = message.readUInt16LE(0);        // 2 bytes
//         const s_identifierId = message.readUInt32LE(2);    // 4 bytes
//         const s_packetType = message.readUInt8(6);         // 1 byte
//         const s_sequenceNumber = message.readUInt16LE(7);  // 2 bytes
//         const s_publicKey = message.slice(9, 9 + 32);      // 32 bytes
//         const s_authTag = message.slice(41, 41 + 16);      // 16 bytes
//         // Log the parsed values.
//         console.log("Parsed Handshake Frame:");
//         console.log("  Preamble:       0x" + s_preamble.toString(16));
//         console.log("  Identifier ID:  0x" + s_identifierId.toString(16));
//         console.log("  Packet Type:    0x" + s_packetType.toString(16));
//         console.log("  Sequence Number:", s_sequenceNumber);
//         console.log("  Public Key:", s_publicKey.toString('hex'));
//         console.log("  Auth Tag: ", s_authTag.toString('hex'));
//     }
// });
