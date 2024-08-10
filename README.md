# AIoT for Remote Healthcare System (Update 04/08/2024)

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
--------------------------------------------------
* What is synchronize between Device and Server ?
* ACK (Handshaking).
* https://en.wikipedia.org/wiki/Ascon_(cipher)
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





  
