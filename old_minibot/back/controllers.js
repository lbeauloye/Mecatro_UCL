const { exec } = require('child_process');
const { db, data } = require('./models/db');

function wesh(req, res){
    res.send('wesh')
}

function handleManualCtrl(req, res){
    let speedR, speedL, order;

    if(req.body.sync){
        speedR = req.body.speedL * 0.99;
        speedL = req.body.speedL * 0.99;
    } else {
        speedL = req.body.speedL * 0.99;
        speedR = req.body.speedR * 0.99;
    }

    switch(req.body.order){
        case 'UP':
            order = 'UP';
            speedR = - speedR;
            speedL = - speedL;
            break;

        case 'LEFT':
            order = 'UP';
            speedL = - speedL;
            break;

        case 'RIGHT':
            order = 'UP';
            speedR = - speedR;
            break;

        case 'DOWN':
            order = 'UP';
            break;

        case 'NO':
            order = 'NO';
            break;

        default:
            order = 'NO';
            break;

    }

    console.log(speedR);
    console.log(speedL);
    res.send('ok')
    exec(`../redAlert manual ${order} ${speedR} ${speedL}`, (err, stdout, stderr) => {
        if (err) {
            console.log(err);
        } else {
          console.log(stdout);
        }
      });

}

function handleKill(req,res){
    exec(`killall -9 redAlert`, (err, stdout, stderr) => {
        res.send('ok')
        exec(`../redAlert stop`, (err, stdout, stderr) => {
            if (err) {
                console.log(err);
            } else {
              console.log(stdout);
            }
      });
  });

    
}

function handleBeacon(req,res){
    exec(`killall -9 redAlert`, (err, stdout, stderr) => {
            res.send('ok')
          if(req.body.order === 'drunk'){
            exec(`../redAlert beacon drunk > /dev/null 2>/dev/null &`, (err, stdout, stderr) => {
                if (err) {
                    console.log(err);
                } else {
                  console.log(stdout);
                }
              });
          } else {
            exec(`../redAlert beacon > /dev/null 2>/dev/null &`, (err, stdout, stderr) => {
                if (err) {
                    console.log(err);
                } else {
                  console.log(stdout);
                }
              });
          }
        
      });
}

function getData(req,res){
    data.findAll({
        limit: 500,
        offset: req.body.offset,
        order: [
            ['id', 'DESC'],
        ],
    }).then((dat)=>{
        const speedL = [];
        const speedR = [];
        dat.forEach((dd)=>{
            speedL.push({
                'x': dd.dataValues.time,
                'y': dd.dataValues.speedL
            })
            speedR.push({
                'x': dd.dataValues.time,
                'y': dd.dataValues.speedR
            })
        })
        res.send({
            'speedL': speedL,
            'speedR': speedR
        })
    })
}

module.exports = {
    wesh,
    handleManualCtrl,
    handleBeacon,
    handleKill,
    getData,
}