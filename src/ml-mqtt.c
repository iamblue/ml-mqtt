#include <string.h>
#include "jerry-api.h"
#include "microlattice.h"
#include "MQTTClient.h"
#include "./mqtt.h"

static int arrivedcount = 0;
Client c;   //MQTT client
char topic_buffer[100];
MQTTMessage message;
int rc = 0;

DELCARE_HANDLER(__mqttClient) {
  arrivedcount = 0;

  unsigned char msg_buf[100];     //generate messages such as unsubscrube
  unsigned char msg_readbuf[100]; //receive messages such as unsubscrube ack

  Network n;  //TCP network
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

  //init mqtt network structure
  NewNetwork(&n);

  jerry_size_t port_req_sz = jerry_get_string_size (args_p[1]);
  jerry_char_t port_buffer[port_req_sz];
  jerry_string_to_char_buffer (args_p[1], port_buffer, port_req_sz);
  port_buffer[port_req_sz] = '\0';

  jerry_size_t server_req_sz = jerry_get_string_size (args_p[0]);
  jerry_char_t server_buffer[server_req_sz];
  jerry_string_to_char_buffer (args_p[0], server_buffer, server_req_sz);
  server_buffer[server_req_sz] = '\0';

  rc = ConnectNetwork(&n, server_buffer, port_buffer);

  if (rc != 0) {
    printf("TCP connect fail,status -%4X\n", -rc);
    return true;
  }

  //init mqtt client structure
  MQTTClient(&c, &n, 12000, msg_buf, 100, msg_readbuf, 100);

  jerry_size_t clientId_req_sz = jerry_get_string_size (args_p[3]);
  jerry_char_t clientId_buffer[clientId_req_sz];
  jerry_string_to_char_buffer (args_p[3], clientId_buffer, clientId_req_sz);
  clientId_buffer[clientId_req_sz] = '\0';

  //mqtt connect req packet header
  data.willFlag = 0;
  data.MQTTVersion = 3;
  data.clientID.cstring = clientId_buffer;
  data.username.cstring = NULL;
  data.password.cstring = NULL;
  data.keepAliveInterval = 10;
  data.cleansession = 1;

  //send mqtt connect req to remote mqtt server
  rc = MQTTConnect(&c, &data);

  if (rc != 0) {
    printf("MQTT connect fail,status%d\n", rc);
  }

  void messageArrived(MessageData *md) {
    MQTTMessage *message = md->message;

    jerry_value_t params[0];
    params[0] = jerry_create_string(message->payload);

    jerry_value_t this_val = jerry_create_undefined();
    jerry_value_t ret_val = jerry_call_function (args_p[5], this_val, &params, 1);

    jerry_release_value(params);
    jerry_release_value(this_val);
    jerry_release_value(ret_val);
  }

  jerry_size_t topic_req_sz = jerry_get_string_size (args_p[2]);
  jerry_char_t topic_buffer[topic_req_sz];
  jerry_string_to_char_buffer (args_p[2], topic_buffer, topic_req_sz);
  topic_buffer[topic_req_sz] = '\0';

  printf("Subscribing to %s\n", topic_buffer);

  switch ((int) jerry_get_number_value(args_p[4])) {
    case 0:
      rc = MQTTSubscribe(&c, topic_buffer, QOS0, messageArrived);
      break;
    case 1:
      rc = MQTTSubscribe(&c, topic_buffer, QOS1, messageArrived);
      break;
    case 2:
      rc = MQTTSubscribe(&c, topic_buffer, QOS2, messageArrived);
      break;
  }

  while (arrivedcount < 1) {
    MQTTYield(&c, 1000);
  }

  free(server_buffer);
  free(topic_buffer);
  free(port_buffer);
  free(clientId_buffer);

  return jerry_create_boolean(true);
}

DELCARE_HANDLER(__mqttClose) {
  arrivedcount = 1;
  return jerry_create_boolean(true);
}

DELCARE_HANDLER(__mqttSend) {
  char buf[100];

  jerry_size_t msg_req_sz = jerry_get_string_size (args_p[0]);
  jerry_char_t msg_buffer[msg_req_sz];
  jerry_string_to_char_buffer (args_p[0], msg_buffer, msg_req_sz);
  msg_buffer[msg_req_sz] = '\0';

  switch ((int) jerry_get_number_value(args_p[1])) {
    case 0:
      message.qos = QOS0;
      break;
    case 1:
      message.qos = QOS1;
      break;
    case 2:
      message.qos = QOS2;
      break;
  }

  message.retained = false;
  message.dup = false;
  message.payload = (void *)msg_buffer;
  message.payloadlen = strlen(msg_buffer) + 1;
  rc = MQTTPublish(&c, topic_buffer, &message);

  return jerry_create_boolean(true);
}

void ml_mqtt_init(void) {
  REGISTER_HANDLER(__mqttClient);
  REGISTER_HANDLER(__mqttClose);
  REGISTER_HANDLER(__mqttSend);
}