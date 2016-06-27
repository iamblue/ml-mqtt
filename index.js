module.exports.client = function(host, port, topic, clientId, qos, cb) {
  return __mqttClient(host, port, topic, clientId, qos, cb);
};
module.exports.send = function(data, qos) {
  return __mqttSend(data, qos);
};
module.exports.close = function() {
  return __mqttClose();
};
