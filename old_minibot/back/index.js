const express = require('express');
const bodyParser = require('body-parser');
const router = require('./routers');
const cors = require('cors');

const app = express();

const {db} = require('./models/db');

app.use(cors());


app.use(bodyParser.json());

//Routes
app.get("/", (req, res) => res.send("hello"));
app.use(router)

app.listen(3001, () => console.log(`REST Server running on port 3001`));
