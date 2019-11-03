//
// Created by otavio on 11/3/19.
//

#ifndef SMART_PC_CONTROL_CLIENT_H
#define SMART_PC_CONTROL_CLIENT_H

#include <gru-0/common/gru_status.h>
#include <gru-0/log/gru_logger.h>
#include <gru-0/network/gru_uri.h>

#include <MQTTClient.h>

#include <stdio.h>

#define QOS_AT_MOST_ONCE 1

typedef struct mqtt_conn_t_ {
    MQTTClient client;
} mqtt_conn_t;

typedef struct reply_t_ {
    char *topic;
    void *payload;
    int len;
} reply_t;

typedef reply_t *(*smart_client_receive_fn)(const char *topic, const void *payload, int len, gru_status_t *status);
typedef gru_status_t (*smart_client_send_fn)(reply_t *reply);

gru_export gru_status_t smart_client_connect(const char *connect_url, const char *id);
gru_export gru_status_t smart_client_subscribe(const char *topic);
gru_export gru_status_t smart_client_receive(smart_client_receive_fn receive_callback, smart_client_send_fn send_callback);
gru_export gru_status_t smart_client_send(reply_t *reply);




#endif //SMART_PC_CONTROL_CLIENT_H
