// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { getAnalytics } from "firebase/analytics";
import { getAuth } from 'firebase/auth';

const firebaseConfig = {
    apiKey: "AIzaSyAtBUaAZXUJhJ-iaex7orGSng4SXkuReSM",
    authDomain: "central-cloud-for-aiot.firebaseapp.com",
    projectId: "central-cloud-for-aiot",
    storageBucket: "central-cloud-for-aiot.appspot.com",
    messagingSenderId: "556407845718",
    appId: "1:556407845718:web:618387e53947a77c911cf7",
    measurementId: "G-3W9K4ZE463"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const analytics = getAnalytics(app);

const auth = getAuth(app);

export { auth };