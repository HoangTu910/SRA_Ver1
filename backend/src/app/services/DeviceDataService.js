const { firestore, database } = require('firebase-admin');
const { DeviceDataCollection } = require('./Collection');
const { UserCollection } = require('./Collection');
const { db, FieldValue } = require('../../config/firebase');
const { setDoc, doc } = require('firebase/firestore');


class DeviceDataService {
  async createDeviceData(data, deviceID) {
      try {
        console.log('Attempting to create heartrate data:', data);
        const docRef = DeviceDataCollection.doc(deviceID);
        await docRef.set({
          ...data,
          createdAt: firestore.FieldValue.serverTimestamp(),
        });
        console.log('Data created with ID:', docRef.id);
        return docRef.id;
      } catch (error) {
        console.log('Error creating data:', error);
        throw error;
      }
    };

  async deleteDeviceDataById(docId) {
    try {
      await firestore.collection('Heartrate').doc(docId).delete();
    } catch (error) {
      console.error('Error deleting document:', error);
      throw error;
    }
  };

  async deleteHeartrateByCondition(condition){
    try {
      const batch = db.batch();
      const querySnapshot = await HeartrateCollection.where('someField', '==', condition).get();
      querySnapshot.forEach((doc) => {
        batch.delete(doc.ref);
      });
      await batch.commit();
    } catch (error) {
      console.error('Error deleting documents:', error);
      throw error;
    }
  };

  async deleteAllDataDeviceDocuments() {
    try {
      const batchSize = 10;
      let querySnapshot = await DeviceDataCollection.limit(batchSize).get();
      
      while (!querySnapshot.empty) {
        const batch = db.batch();
        querySnapshot.docs.forEach((doc) => {
          batch.delete(doc.ref);
        });
        await batch.commit();
        querySnapshot = await DeviceDataCollection.limit(batchSize).get();
      }
    } catch (error) {
      console.error('Error deleting all documents:', error);
      throw error;
    }
  };

  async getDataFromDatabase(userId) {
    try {
      const batchSize = 1; // Number of documents to retrieve in each batch
      const userDocRef = UserCollection.doc(userId); // Reference to the specific user document
      const deviceDataCollection = userDocRef.collection('users'); // Reference to the subcollection within the user document
  
      let querySnapshot = await deviceDataCollection.limit(batchSize).get(); // Get the first batch
      const results = []; // Array to store retrieved documents
  
      while (!querySnapshot.empty) {
        querySnapshot.docs.forEach((doc) => {
          results.push({ id: doc.id, ...doc.data() }); // Add document data to results array
        });
  
        // Fetch the next batch of documents
        querySnapshot = await deviceDataCollection
          .limit(batchSize)
          .startAfter(querySnapshot.docs[querySnapshot.docs.length - 1])
          .get();
      }
      console.log('Data retrieved successfully.');
      return results; // Return the array of document data
    } catch (error) {
      console.error('Error retrieving data from Firestore:', error);
      throw error; // Rethrow error to be handled by the caller
    }
  }

  async createDeviceInFirestore(deviceId, deviceData) {
    try {
      console.log('Attempting to create data:', deviceData);
      const docRef = DeviceDataCollection.doc(deviceId); // Use deviceId as the document ID
      await docRef.set({
        ...deviceData, // Spread the deviceData object into the document
        createdAt: firestore.FieldValue.serverTimestamp(), // Add the createdAt field with a server timestamp
      });
      console.log('Data created with ID:', docRef.id);
      return docRef.id;
    } catch (error) {
      console.log('Error creating data:', error);
      throw error;
    }
  };

  async uploadDeviceInUser(userId, deviceId) {
    try {
      console.log('Attempting to create data for user:', userId);
      const docRef = UserCollection.doc(userId); // Use deviceId as the document ID
      await docRef.set({
        deviceId: deviceId.toString(), // Add deviceId as a field in the document
        createdAt: firestore.FieldValue.serverTimestamp(), // Add createdAt field with a server timestamp
      }, { merge: true });
      console.log('Device data added to user document');
      return { userId, deviceId };
    } catch (error) {
      console.log('Error adding device data to user document:', error);
      throw error;
    }
  };

  async getDataSensorFromDatabase(deviceId){
    try {
      const batchSize = 1; // Number of documents to retrieve in each batch
      const userDocRef = DeviceDataCollection.doc(deviceId); // Reference to the specific user document
      const deviceDataCollection = userDocRef.collection('devices'); // Reference to the subcollection within the user document
  
      let querySnapshot = await deviceDataCollection.limit(batchSize).get(); // Get the first batch
      const results = []; // Array to store retrieved documents
  
      while (!querySnapshot.empty) {
        querySnapshot.docs.forEach((doc) => {
          results.push({ id: doc.id, ...doc.data() }); // Add document data to results array
        });
  
        // Fetch the next batch of documents
        querySnapshot = await deviceDataCollection
          .limit(batchSize)
          .startAfter(querySnapshot.docs[querySnapshot.docs.length - 1])
          .get();
      }
      console.log('Data retrieved successfully.');
      return results; // Return the array of document data
    } catch (error) {
      console.error('Error retrieving data from Firestore:', error);
      throw error; // Rethrow error to be handled by the caller
    }
  };
}
module.exports = new DeviceDataService()
