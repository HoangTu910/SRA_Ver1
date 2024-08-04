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

## Overview System
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

### Frontend setup (React + Vite + Tailwind)
Modify only App.jsx and pages folder when working with frontend
1. Install dependencies
```bash
$ npm i name_package
```
2. Build Frontend
```bash
$ npm run dev
```

**App.jsx Configuration**: 
* Add path by using path="/path_name" and element={<element_name/>}></Route>
* Everytime want to create a new path (API) just add another <Route> inside <Routes>

**Pages Folder**:
* element_name in App.jsx depend on .jsx file in pages
* If want to add new path, create new .jsx file and config html like login.jsx
* If copy a template on Tailwind rememeber to rename the *function* and *export default* 

**Query?**:
* Synchronize user Id and device ID in Database.
* Old data of user? Queue [200] - Database update() or set()?
* Doctor feature.
* Developer or User.


  
