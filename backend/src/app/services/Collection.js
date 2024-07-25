const { db } = require('../../config/firebase');

const HeartrateCollection = db.collection('heartrate');
const Spo2Collection = db.collection('spo2');

module.exports = { HeartrateCollection };