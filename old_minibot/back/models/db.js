const Sequelize = require('sequelize');

const db = new Sequelize('redDb', 'redAlert', 'redAlert', {
  host: 'localhost',
  dialect: 'sqlite',
  operatorsAliases: false,
  storage: 'database.sqlite',
  pool: {
    max: 5,
    min: 0,
    acquire: 30000,
    idle: 10000
  },
});

const data = db.import('data', require('./data'));

db.authenticate()
  .then(() => {
    console.log('Connection has been established successfully.');
  })
  .catch(err => {
    console.error('Unable to connect to the database:', err);
  });

  db.sync();

module.exports = { db, data };
