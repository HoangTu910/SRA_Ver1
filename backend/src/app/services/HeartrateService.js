const { firestore, database } = require('firebase-admin');
const { HeartrateCollection } = require('./Collection');
const { db, FieldValue } = require('../../config/firebase');


class HeartrateService {
  async createHeartrate(heartrate) {
      try {
        const docRef = HeartrateCollection.doc();
        await docRef.set({
          ...heartrate,
          createdAt: firestore.FieldValue.serverTimestamp(),
        });
        return docRef.id;
      } catch (error) {
        console.error('Error creating heart rate data:', error);
        throw error;
      }
    };

  async deleteHeartrateById(docId) {
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

  async deleteAllHeartrateDocuments() {
    try {
      const batchSize = 10;
      let querySnapshot = await HeartrateCollection.limit(batchSize).get();
      
      while (!querySnapshot.empty) {
        const batch = db.batch();
        querySnapshot.docs.forEach((doc) => {
          batch.delete(doc.ref);
        });
        await batch.commit();
        querySnapshot = await HeartrateCollection.limit(batchSize).get();
      }
    } catch (error) {
      console.error('Error deleting all documents:', error);
      throw error;
    }
  };
}
module.exports = new HeartrateService()
