# ml-mqtt

## API

``` js

__mqttClient(
  host,                 // string
  port,                 // string
  topic,                // string
  clientId,             // string
  qos,                  // number: Qo0: 0, Qo1: 1, Qo2: 2
  recieve msg callback, // function
)

```

## Example

``` js

var EventEmitter = require('ml-event').EventEmitter;
var eventStatus = new EventEmitter();
global.eventStatus = eventStatus;

global.eventStatus.on('mqttReceive', function(data) {
  print(data);
});

__mqttClient(
  'test.mosquitto.org',
  '1883',
  '7687test',
  'mqtt-7687-client',
  1,
  function(data) {
    return global.eventStatus.emit('mqttReceive', data);
  }
);

```
