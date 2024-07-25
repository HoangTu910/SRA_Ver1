const heartrateRoute = require('./Heartrate')
const authRoute = require('./Authentication')
const bodyParser = require('body-parser');
const cors = require('cors');

function route(app){
    app.use('/api/user', authRoute)
    app.use('/api/heartrate', heartrateRoute)
}

module.exports = route