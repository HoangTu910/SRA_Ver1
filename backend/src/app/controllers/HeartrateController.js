const HeartrateService = require('../services/HeartrateService');

const {
  deleteHeartrateById,
  deleteHeartrateByCondition,
  deleteAllHeartrateDocuments,
} = require('../services/HeartrateService');

exports.createHeartrate = async (req, res) => {
  try {
    const heartrateData = req.body;
    const docId = await HeartrateService.createHeartrate(heartrateData);
    res.status(200).send({ id: docId });
  } catch (error) {
    res.status(500).send({ message: 'Failed to create heartrate record', error: error.message });
  }
};

exports.deleteHeartrateByIdController = async (req, res) => {
  try {
    const { id } = req.params;
    await deleteHeartrateById(id);
    res.status(200).send({ message: `Document with ID ${id} deleted successfully.` });
  } catch (error) {
    res.status(500).send({ error: 'Failed to delete document' });
  }
};

exports.deleteHeartrateByConditionController = async (req, res) => {
  try {
    const { condition } = req.body;
    await deleteHeartrateByCondition(condition);
    res.status(200).send({ message: 'Documents deleted successfully.' });
  } catch (error) {
    res.status(500).send({ error: 'Failed to delete documents' });
  }
};

exports.deleteAllHeartrateDocumentsController = async (req, res) => {
  try {
    await deleteAllHeartrateDocuments();
    res.status(200).send({ message: 'All documents deleted successfully.' });
  } catch (error) {
    res.status(500).send({ error: 'Failed to delete all documents' });
  }
};