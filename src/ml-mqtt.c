#include <string.h>
#include "jerry.h"
#include "microlattice.h"
#include "MQTTClient.h"
#include "./mqtt.h"

static int arrivedcount = 0;
Client c;   //MQTT client
char topic_buffer[100];
MQTTMessage message;
int rc = 0;

#define clientId_buffer  "mqtt-7687-client"

DELCARE_HANDLER(__mqttClient) {
  char rcv_buf_old[100] = {0};
  arrivedcount = 0;
  /* server */
  int server_req_sz = -jerry_api_string_to_char_buffer (args_p[0].v_string, NULL, 0);
  char * server_buffer = (char*) malloc (server_req_sz);
  server_req_sz = jerry_api_string_to_char_buffer (args_p[0].v_string, (jerry_api_char_t *) server_buffer, server_req_sz);
  server_buffer[server_req_sz] = '\0';

  // /* port */
  int port_req_sz = -jerry_api_string_to_char_buffer (args_p[1].v_string, NULL, 0);
  char * port_buffer = (char*) malloc (port_req_sz);
  port_req_sz = jerry_api_string_to_char_buffer (args_p[1].v_string, (jerry_api_char_t *) port_buffer, port_req_sz);
  port_buffer[port_req_sz] = '\0';

  // /* topic */
  int topic_req_sz = -jerry_api_string_to_char_buffer (args_p[2].v_string, NULL, 0);
  char * topic_buffer = (char*) malloc (topic_req_sz);
  topic_req_sz = jerry_api_string_to_char_buffer (args_p[2].v_string, (jerry_api_char_t *) topic_buffer, topic_req_sz);
  topic_buffer[topic_req_sz] = '\0';

  // /* clientId */
  // int clientId_req_sz = -jerry_api_string_to_char_buffer (args_p[3].v_string, NULL, 0);
  // char * clientId_buffer = (char*) malloc (clientId_req_sz);
  // clientId_req_sz = jerry_api_string_to_char_buffer (args_p[3].v_string, (jerry_api_char_t *) clientId_buffer, clientId_req_sz);
  // clientId_buffer[clientId_req_sz] = '\0';

  /* tls */
  // printf("topic: %s\n", topic_buffer);
  // printf("clientId: %s\n", clientId_buffer);
  // printf("server_buffer: %s\n", server_buffer);
  // printf("port_buffer: %s\n", port_buffer);

  unsigned char msg_buf[100];     //generate messages such as unsubscrube
  unsigned char msg_readbuf[100]; //receive messages such as unsubscrube ack

  Network n;  //TCP network
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

  //init mqtt network structure
  NewNetwork(&n);

  rc = ConnectNetwork(&n, server_buffer, port_buffer);

  if (rc != 0) {
    printf("TCP connect fail,status -%4X\n", -rc);
    return true;
  }

  //init mqtt client structure
  MQTTClient(&c, &n, 12000, msg_buf, 100, msg_readbuf, 100);

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

  printf("Subscribing to %s\n", topic_buffer);

  void messageArrived(MessageData *md) {
    MQTTMessage *message = md->message;
    char rcv_buf[100] = {0};
    strcpy(rcv_buf, message->payload);

    if (strcmp(rcv_buf_old, rcv_buf) != 0) {
      rcv_buf[(size_t)(message->payloadlen)] = '\0';

      jerry_api_value_t params[0];
      params[0].type = JERRY_API_DATA_TYPE_STRING;
      params[0].v_string = jerry_api_create_string (rcv_buf);

      * rcv_buf_old = "";
      strcpy(*rcv_buf_old, rcv_buf);

      jerry_api_call_function(args_p[5].v_object, NULL, false, &params, 1);
      jerry_api_release_value(&params);
    }
  }

  switch ((int)args_p[4].v_float32) {
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

  printf("Client Subscribed %d\n", rc);

  for(;;) {
    MQTTYield(&c, 1000);
  }

  ret_val_p->type = JERRY_API_DATA_TYPE_BOOLEAN;
  ret_val_p->v_bool = true;
  free(server_buffer);
  free(topic_buffer);
  free(port_buffer);
  free(clientId_buffer);
  return true;
}

DELCARE_HANDLER(__mqttClose) {
  arrivedcount = 1;
  return true;
}

DELCARE_HANDLER(__mqttSend) {
  char buf[100];

  // int msg_req_sz = jerry_api_string_to_char_buffer(args_p[0].v_string, NULL, 0);
  // msg_req_sz *= -1;
  // char msg_buffer [msg_req_sz+1]; // 不能有*
  // msg_req_sz = jerry_api_string_to_char_buffer (args_p[0].v_string, (jerry_api_char_t *) msg_buffer, msg_req_sz);
  // msg_buffer[msg_req_sz] = '\0';

  int msg_req_sz = -jerry_api_string_to_char_buffer (args_p[0].v_string, NULL, 0);
  char * msg_buffer = (char*) malloc (msg_req_sz);
  msg_req_sz = jerry_api_string_to_char_buffer (args_p[0].v_string, msg_buffer, msg_req_sz);
  msg_buffer[msg_req_sz] = '\0';

  switch ((int)args_p[1].v_float32) {
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
  return true;
}

void ml_mqtt_init(void) {
  REGISTER_HANDLER(__mqttClient);
  REGISTER_HANDLER(__mqttClose);
  REGISTER_HANDLER(__mqttSend);
}