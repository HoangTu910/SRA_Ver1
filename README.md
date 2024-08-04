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

**Query?**:
* Synchronize user Id and device ID in Database.
* Old data of user? Queue [200] - Database update() or set()?
* Doctor feature.
* Developer or User.


  
