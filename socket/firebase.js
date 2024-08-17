var admin = require("firebase-admin");

const {getFirestore} = require('firebase-admin/firestore')

var serviceAccount = require("./central-cloud-for-aiot-firebase-adminsdk-beycw-c3f8f3a152.json");

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount)
  
});

const db = getFirestore();
const auth = admin.auth();

module.exports = {db, auth, admin};
