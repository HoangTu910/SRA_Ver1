var admin = require("firebase-admin");

const {getFirestore} = require('firebase-admin/firestore')

var serviceAccount = require("./cloud-aiot-firebase-adminsdk-ajjtt-3c3bd26b44.json");

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount)
  
});

const db = getFirestore();
const auth = admin.auth();

module.exports = {db, auth, admin};
