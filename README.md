# AIoT for Remote Healthcare System (Update 27/08/2024)

## General System

### Overview
Providing remote health monitoring solutions including multiple functions such as:
* Monitor continously health indicators.
* Using AI model to predict current health state of user.
* Provides an interface to display health infomation.
The system will focus on server management system, information security, optimize user interface and apply Artificial Intelligent. Implement basic device for collecting user data through sensor and process data in server.

## Phase 1 - Getting Started
- **Literature Overview**: Determine which system is appropriate.
- **System Design**: Start to implement the overview system.
- **Firebase reseacrh**: Research how to use Firebase for authenticating and data management.
- **Backend Technology**: Research multiple technology suitable for the system.

## System Design
The overview of the system display below. Each part in the system will have it specific design (will be explained detail in other part). 
![ảnh](https://github.com/user-attachments/assets/441f1d63-33ae-4a7e-a6d3-49fcb573ed70)

### Firebase server setup

1. Build webpack
```bash
$ node_modules/.bin/webpack
```
2. Start server
```bash
$ serve -n dist/
```

### User Interface (Front-end)
The front-end including a single dashboard display current data in realtime. Also it contain some charts and prediction zone for AI implementation. Until now, there two main UI used in the system (Login Page and Dashboard). 
Technology used for front-end:
* ReactJS.
* Tailwind CSS.
* Material UI.
The overview front-end display below:
![ảnh](https://github.com/user-attachments/assets/6aea23cb-41c5-40ff-b792-343b6f47f0cc)
![ảnh](https://github.com/user-attachments/assets/546b5479-fe2b-45f9-bdf2-b949f7aa3fc4)

1. Install dependencies
```bash
$ npm i name_package
```
2. Build Frontend
```bash
$ npm run dev
```

**App.jsx Configuration**: Add path by using path="/path_name" and element={<element_name/>}></Route>. Everytime want to create a new path (API) just add another <Route> inside <Routes>

**Pages Folder**: element_name in App.jsx depend on .jsx file in pages. If want to add new path, create new .jsx file and config html like login.jsx. If copy a template on Tailwind rememeber to rename the *function* and *export default*.

## Phase 2: Server Implementation

### Server-side (Backend)
In server of the system contain three main components: MQTT Broker, Socket and Backend (Data processing layer).

**1. MQTT Broker**

The reason of using custom MQTT Broker instead of public MQTT Broker is the custom broker provides faster response than public MQTT Broker. When using public MQTT Broker, every time device publish a topic, it took server about 20-30 second to subscribe to that topic. Custom broker can make the publish and subscribe between server and device instantly. Futhermore, custom MQTT Broker allow developer to config or set up authentication and encryption which make the data transfer securable. Aedes is used to build the custom MQTT Broker. 

![ảnh](https://github.com/user-attachments/assets/06e2f276-8c04-4465-9975-caf9ac5d4b16)

**2. Backend Server**

The backend was built based on Nodejs express. The database used in the system is Firebase. The reason why firebase because it suitable for a project, it provide 50000 times of write, 20000 times of read in database everyday (MongoDB allow 1M times but once). The backend system handle the authentication, communication with the MQTT Broker and also it provide multiple API for data processing. There are two collections in the database, details will display below:

![ảnh](https://github.com/user-attachments/assets/9a000a74-dd6b-4cfa-8c45-8c5a2ac06df8)
![ảnh](https://github.com/user-attachments/assets/5782da00-5e08-45aa-b749-228d420e7780)
![ảnh](https://github.com/user-attachments/assets/09eeb00c-0d42-4433-b262-f3587d506a5c)

When start the backend, it will connect to custom MQTT Broker by default username and password setted up in backend server and subscribe to a topic on broker.

![ảnh](https://github.com/user-attachments/assets/ca728bef-0499-481e-9106-b3f9ba61241e)

**Authentication**: The backend handle the session of each user by verifying token. Make sure everytime user access to the dashboard, user need to login again to generate unique token. The image below display the result after finished authentication for user. It generated a unique token for each session. The user ID was console log to verify if the user is correctly login.

![ảnh](https://github.com/user-attachments/assets/2d2be85a-6d8b-4154-96b1-f8bf96f97c74)


**Query?**:
* Synchronize user Id and device ID in Database (Finished).
* Old data of user? Queue [200] - Database update() or set() (update currently)?
* Doctor feature (Yet).
* Developer or User (User).
* Time series database.
* Cryptography benchmark.
--------------------------------------------------
* What is synchronize between Device and Server ?
* ACK (Handshaking).
* https://nodejs.org/api/child_process.html - NodeJS Child Process
* https://en.wikipedia.org/wiki/Ascon_(cipher) - New Cryptography
* Reconstruct the frame protocol? JSON or transfer frame directly to server (if yes how? because the frame is too large to handle).
* Encryption implementation in server side.
* Dockerize the whole project (created into 4 seperate folder so don't know how to run it the same time).
* Some handling function in frame construct and parsing process.
* Project structure.
### Encryption process:
**ESP32 Gateway sent**
![ảnh](https://github.com/user-attachments/assets/68cf7617-6ed2-445e-90b7-a91cf55cd861)
**Server received**
![ảnh](https://github.com/user-attachments/assets/3b32e905-cde8-4126-ba09-be1d982466f1)


## Ascon Cryptography (Ascon-128a AEAD)
*Source: https://github.com/ascon/ascon-c*

*Comparision with Chacha20-poly1305 (ARM-Cotex M3): https://crypto.stackexchange.com/questions/89990/nist-lwc-finalists-aead-vs-chacha20-poly1305*

![ảnh](https://github.com/user-attachments/assets/69a8b46a-7f7c-4b1e-abc1-aa1a87c6eb3d)

### Ascon-128a AEAD structure
![ảnh](https://github.com/user-attachments/assets/4719acd8-beeb-45bc-a3be-caf25a9fbd5f)
1. Key: 128-bit
2. Nonce: 128-bit
3. Tag: 128-bit
4. Asscociated Data: Optional
5. Plaintext: Data need to be encrypted
6. Ciphertext: Encrypted data

### Encrypt Process
**1. Initialization**
* Load key and nonce into internal register of Ascon (State Register) and inform the initilization state
![ảnh](https://github.com/user-attachments/assets/eaa189d2-a8fa-4c19-8643-4f156b5271a4)

**2. Process Association Data**
* The Association Data (AD) will be processed through multiple transformation (ascon_duplex()). After the process, a simple XOR operation will be use to prepare for the plaintext processing.
![ảnh](https://github.com/user-attachments/assets/2ed2b416-80ad-4646-bd9f-2253bd5d8975)

**3. Encryption**
* Plaintext will be XOR with the internal state to generate the ciphertext. Everytime it process a block, the internal state will be updated through some non-linear transformation.

**4. Finalization**
* The secret key will be XOR with internal state to make sure if any change in encryption process occur, it will affect to the final result. Finally, a 128-bit tag will be generated. The tag will be sent along with ciphertext to make sure data does not change in transmission process.

## Frame Structure

### Structure
Update ACK for three-way handshake and device ID data type.

![ảnh](https://github.com/user-attachments/assets/b21512be-00a7-4fd9-9a1d-7f83bff4c35a)

### Three-way handshake implemetation between ESP32 and STM32 (without frame)

![ảnh](https://github.com/user-attachments/assets/e352bdd8-1b0d-47cb-9859-8346eacf3096)

### Combined with frame (parsing from ESP32 - without encrytion)

![ảnh](https://github.com/user-attachments/assets/1df86a02-79fe-4b70-b8d3-85bf4ce90566)

### Encrypt data packet

![ảnh](https://github.com/user-attachments/assets/d8da38c0-ede3-4352-9b54-6a1459b6dace)

### Three way handshake between ESP32 Gateway and API Server
After construct a new struct *encrypted_struct*, the new struct will be convert into JSON base to send to server. Before send it to server, gateway will publish a ACK command on *handshake/ack* topic, the server will subscribe to that topic and publish SYN_ACK command on *handshake/syn_ack* topic. The gateway will subscribe to that topic and verify, if correct ESP32 will publish the final ACK and establish the connection. The JSON will be sent to server for parsing and process.

* ESP32 send ACK for initiating three-way handshake
![ảnh](https://github.com/user-attachments/assets/0ffac51d-d26f-4bdd-9095-e70b23d585ac)

* Server response and receive JSON package
![ảnh](https://github.com/user-attachments/assets/0717599e-43b3-4fdf-a85f-52df07edeb7f)

* Error if cannot establish three-way handshake between gateway and server
![ảnh](https://github.com/user-attachments/assets/eb05064c-37cc-495b-9e92-6563d6a13b40)

### Decryption server side
For decryption, the *child_process* library will be used. In backend server will contain the C++ source of encryption and decryption. A makefile will be created for compile the whole C source file, when npm start the server, package.json will run the makefile to generate executable file of decryption. After the server received the full JSON data from gateway, it will pass the encrypted data into the process for decryption

* Function to run the process
  
![ảnh](https://github.com/user-attachments/assets/9e8e1f4d-3ab6-4a14-a2eb-3caadd0b8fb6)

* Makefile to compile C source file
  
![ảnh](https://github.com/user-attachments/assets/32e36f59-3ce5-4d1a-9f91-84daab2aa738)

* Result after decryption
  
![ảnh](https://github.com/user-attachments/assets/027fa450-7e84-4ade-a7a3-eb88e658e10b)

## Fully operation of the system

* The STM32 init the three way handshake with ESP32 by sending SYN command. After completed verify the three way handshake, STM32 start to send to full frame with the strucutre explained above to ESP32. ESP32 will parse the frame and encrypt the data packet using Ascon-128a. ESP32 will construct a new frame and send it to server (it also establish the three-way handshake).
* In server there are three main component backend, socket and broker. Backend and broker has been explained above, the socket is the brigde between server and database, to synchronize the data between device and database (make sure that database update continously and constantly every time data change). Socket will call an API to backend to require data send update the data to database quickly.

*Device Log*

![ảnh](https://github.com/user-attachments/assets/92145541-4ff5-4115-bb18-04c42ee1673e)

*Server log*

![ảnh](https://github.com/user-attachments/assets/2a5281d3-b64e-4b16-88cc-7ac7cca96762)

*Database*

![ảnh](https://github.com/user-attachments/assets/acb63d1f-13c5-4109-bcc5-209157f76654)

*Interface*

![ảnh](https://github.com/user-attachments/assets/48a8496d-0c3d-4e27-acba-db75705f61fa)

## Phase 3: Edge AI implemetation (ESP32)
### Some keywords
* Data compression (RLE, DPCM)
* Data aggregation (Event-based)
* Anomaly Detection (ML - Isolation Forest - Decision Tree) or (ML - Autoencoder - LSTM).

## Example data points
### Resting
* Heart Rate: 60-70 bpm
* Accelerometer: 0.3-1.0 m/s²
* Temperature: 36.3-36.7°C
* Label: Resting
### Walking
* Heart Rate: 85-100 bpm
* Accelerometer: 3-6 m/s²
* Temperature: 36.8-37.1°C
* Label: Walking
### Running
* Heart Rate: 120-140 bpm
* Accelerometer: 10-12 m/s²
* Temperature: 37.2-37.5°C
* Label: Running
### High-Intensity Exercise (Sport)
* Heart Rate: 150-170 bpm
* Accelerometer: 14-18 m/s²
* Temperature: 37.5-38.0°C
* Label: High-Intensity Exercise (Sport)
### Dataset structure

| Heart Rate (bpm) | Accelerometer (m/s²) | Temperature (°C) | Activity State (Label)              |
|------------------|-----------------------|------------------|-------------------------------------|
| 60               | 0.5                   | 36.5             | Resting                             |
| 65               | 0.4                   | 36.4             | Resting                             |
| 90               | 5.2                   | 37.0             | Walking                             |
| 95               | 5.8                   | 37.2             | Walking                             |
| 130              | 10.2                  | 37.5             | Running                             |
| 140              | 12.0                  | 37.8             | Running                             |
| 160              | 15.5                  | 38.0             | High-Intensity Exercise (Sport)     |
| 150              | 14.0                  | 38.1             | High-Intensity Exercise (Sport)     |
| 100              | 3.5                   | 36.9             | Walking                             |
| 65               | 0.4                   | 36.3             | Resting                             |

## Anomaly Detection V1.0

- This model is the combination between Isolation Forest and Neural Networking.
https://colab.research.google.com/drive/1-bmDGz6satWL-ypIJZbpSJl4uA-0ouzw?authuser=2

### Define the specific conditions

- There are 4 condition with specific data and parameters for each condition (define above)

### Apply Isolation Forest

![ảnh](https://github.com/user-attachments/assets/9cc0f092-5153-49c2-90d3-2e454e862dc7)
- The Isolation Forest algorithm is used to detect anomalies in the heart rate data and apply the 'Anomaly' label for each data.

### Apply Neural Networking

- The features (heart rate, accelerometer, temperature) and labels (anomaly detection results) are separated for training the deep learning model.
- The dataset is balanced using SMOTE (Synthetic Minority Over-sampling Technique), which generates synthetic samples for the minority class to address class imbalance issues.

### Conversion to Tensorflow Lite

- After training, the model is converted to TensorFlow Lite format, which allows it to be deployed on devices with limited resources, such as microcontrollers.

## Implement model on ESP32 using Tensorflow Lite

- The model has been successfully implemented on ESP32, the prediction is correct with the given data
- A problem is Flash Memory consume 38.2% in total (the first implementation of gateway is 58.7%) so in total will be 96.6% (predict)
 
![ảnh](https://github.com/user-attachments/assets/f4667c1e-8d3d-4098-915e-2abcd9b59021)

## Bechmark Cryptography
- The tests below were run on an ESP32 Dev Module running at 240MHz base on running each algorithms 1000 times.
### Conclusion
* AES still the best option for ESP32, it perform is outstanding compared to Chacha20 and Ascon-128a on ESP32.
* With block size of 16 bytes and 32 bytes, Ascon-128a given better performance compared to Chacha20. However, with block size of 64 and 128 bytes Chacha20 given better performance compared to Ascon-128a.
### Table of Comparison
### Results for Block Size: 16 bytes

| Algorithm             | Time Taken (seconds) | Clock Cycles Used | Throughput (bytes/sec) | Cycles per Byte | Microseconds per Byte | Memory Used (bytes) |
|-----------------------|-----------------------|-------------------|-------------------------|------------------|----------------------|---------------------|
| ChaCha20-Poly1305     | 0.06                  | 14916726          | 253968.25               | 932.30           | 3.94                 | 0                   |
| AES                   | 0.01                  | 1972761           | 2000000.00              | 123.30           | 0.50                 | 0                   |
| Ascon-128a           | 0.03                  | 7213014           | 533333.33               | 450.81           | 1.87                 | 0                   |

### Results for Block Size: 32 bytes

| Algorithm             | Time Taken (seconds) | Clock Cycles Used | Throughput (bytes/sec) | Cycles per Byte | Microseconds per Byte | Memory Used (bytes) |
|-----------------------|-----------------------|-------------------|-------------------------|------------------|----------------------|---------------------|
| ChaCha20-Poly1305     | 0.06                  | 14777813          | 524590.16               | 461.81           | 1.91                 | 0                   |
| AES                   | 0.01                  | 1936536           | 4000000.00              | 60.52            | 0.25                 | 0                   |
| Ascon-128a           | 0.04                  | 8586188           | 888888.89               | 268.32           | 1.12                 | 0                   |

### Results for Block Size: 64 bytes

| Algorithm             | Time Taken (seconds) | Clock Cycles Used | Throughput (bytes/sec) | Cycles per Byte | Microseconds per Byte | Memory Used (bytes) |
|-----------------------|-----------------------|-------------------|-------------------------|------------------|----------------------|---------------------|
| ChaCha20-Poly1305     | 0.06                  | 14779155          | 1049180.33              | 230.92           | 0.95                 | 0                   |
| AES                   | 0.01                  | 1936620           | 8000000.00              | 30.26            | 0.13                 | 0                   |
| Ascon-128a           | 0.05                  | 11407900          | 1333333.33              | 178.25           | 0.75                 | 0                   |

### Results for Block Size: 128 bytes

| Algorithm             | Time Taken (seconds) | Clock Cycles Used | Throughput (bytes/sec) | Cycles per Byte | Microseconds per Byte | Memory Used (bytes) |
|-----------------------|-----------------------|-------------------|-------------------------|------------------|----------------------|---------------------|
| ChaCha20-Poly1305     | 0.06                  | 14779305          | 2098360.66              | 115.46           | 0.48                 | 0                   |
| AES                   | 0.01                  | 1936578           | 16000000.00             | 15.13            | 0.06                 | 0                   |
| Ascon-128a           | 0.07                  | 17054441          | 1802816.90    


### Chart parameter
![Gemini_Chart_Image_1dhqxa1dhqxa1dhq](https://github.com/user-attachments/assets/e8a1c156-556b-4bca-8f21-2305b48c5daa)
![Gemini_Chart_Image_17ryci17ryci17ry](https://github.com/user-attachments/assets/07e1854d-00aa-4643-9046-48b2bb365a73)
![Gemini_Chart_Image_d5xhrfd5xhrfd5xh](https://github.com/user-attachments/assets/f36ebb02-369b-4b47-8ed7-e780f57cffc2)
![Gemini_Chart_Image_dfuqkgdfuqkgdfuq](https://github.com/user-attachments/assets/0419ee3f-4568-43c0-8247-8b5842a9a7e7)

### Community comparison between Chacha20 and Ascon-128a on ESP32

- Actually it give the similar result as me

![ảnh](https://github.com/user-attachments/assets/325443f0-d2f3-4c31-9024-9641297122c7)



















  
