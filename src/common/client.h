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

typedef gru_status_t (*smart_client_callback_fn)(const char *topic, const void *payload, int len);

gru_export gru_status_t smart_client_connect(char *connect_url);
gru_export gru_status_t smart_client_subscribe(const char *topic);
gru_export gru_status_t smart_client_receive(smart_client_callback_fn callback);




#endif //SMART_PC_CONTROL_CLIENT_H
