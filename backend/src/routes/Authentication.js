const express = require('express');

const AuthController = require('../app/controllers/AuthController');

const router = express.Router();

router.post('/login', AuthController.loginController);
router.post('/signup', AuthController.signUp)

module.exports = router;