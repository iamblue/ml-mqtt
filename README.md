# ml-mqtt

## Required

* ml-wifi

## API

``` js

// Regist MCS.
__mqttClient(
  host,        // string
  port,        // string
  datachannel, // string
  client,      // string
  Qos,         // number
  func,        // callback function, handle the MQTT msg.
)

// Send a datapoint.
__mqttSend(
  topic,       // string
  data,        // string
  Qos,         // number
)

```

## Example

Show a case: Get a message from MCS and send a message to MCS.

``` js

  var sendChannel = 'sendmsg';
  var receiveChannel = 'receivemsg';
  var deviceId = 'Input your deviceId';
  var deviceKey = 'Input your deviceKey';

  var topic = 'mcs/' + deviceId + '/' + deviceKey + '/';

  __wifi({
    mode: 'station', // default is station
    auth: 'PSK_WPA2',
    ssid: 'Input your ssid',
    password: 'Input your password',
  });

  global.eventStatus.on(receiveChannel, function(data) {
    print(data);
    var msg = "hello world";
    return __mqttSend(topic + sendChannel, ',,' + msg , 0);
  });

  global.eventStatus.on('wifiConnect', function() {
    __mqttClient(
      'mqtt.mcs.mediatek.com',
      '1883',
      topic + '+',
      'mqtt-7687-client',
      1,
      function(data) {
        global.eventStatus.emit(data.split(',')[1], data);
      }
    );
  });

```