

module.exports = function(sequelize, DataTypes){
    return sequelize.define('datas', {
      speedL: {
        type: DataTypes.DOUBLE,
        allowNull: false,
      },
      speedR: {
        type: DataTypes.DOUBLE,
        allowNull: false,
      },
      time: {
        type: DataTypes.DOUBLE,
        allowNull: false,
      },
      name: {
        type: DataTypes.STRING(25),
        allowNull: true,
      }
    });
};
