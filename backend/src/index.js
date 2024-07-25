const express = require('express');
const app = express();
const port = 6969;
const cors = require('cors');
const bodyParser = require('body-parser');
const route = require("./routes");

app.use(bodyParser.json());
app.use(cors());
app.use(express.json());

route(app);

// Test route
// app.use("/test", (req, res) => {
//     console.log("Hello world");
//     res.send("Hello world");
// });

app.listen(port, '0.0.0.0', () => {
    console.log(`Server is running on port ${port}`);
});
