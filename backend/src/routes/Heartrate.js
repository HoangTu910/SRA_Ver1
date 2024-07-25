const express = require('express');
const HeartrateController = require('../app/controllers/HeartrateController');

const {
    deleteHeartrateByIdController,
    deleteHeartrateByConditionController,
    deleteAllHeartrateDocumentsController,
  } = require('../app/controllers/HeartrateController');

const router = express.Router();

router.post('/', HeartrateController.createHeartrate);
router.delete('/delete', deleteAllHeartrateDocumentsController);
router.delete('/:id', deleteHeartrateByIdController);
router.delete('/condition', deleteHeartrateByConditionController);

module.exports = router;