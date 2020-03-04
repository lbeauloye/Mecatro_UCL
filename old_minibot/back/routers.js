const express = require('express');
const router = new express.Router();
const controllers = require('./controllers');

router.route('/ctrl/manual').post(controllers.handleManualCtrl);
router.route('/ctrl/beacon').post(controllers.handleBeacon);
router.route('/ctrl/kill').post(controllers.handleKill);
router.route('/datas/speed/').get(controllers.getData);
router.route('/').get(controllers.wesh);

module.exports = router;