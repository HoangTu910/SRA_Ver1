// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { getAnalytics } from "firebase/analytics";
import { getAuth } from 'firebase/auth';

const firebaseConfig = {
  apiKey: "AIzaSyAsuogNa0lKBxy8Hftd6hb-M0HrfguBems",
  authDomain: "cloud-aiot.firebaseapp.com",
  projectId: "cloud-aiot",
  storageBucket: "cloud-aiot.appspot.com",
  messagingSenderId: "293880461791",
  appId: "1:293880461791:web:fe4f6a91bea6cda7932c6a",
  measurementId: "G-CEX6QVDFFR"
};


// Initialize Firebase
const app = initializeApp(firebaseConfig);
const analytics = getAnalytics(app);

const auth = getAuth(app);

export { auth };