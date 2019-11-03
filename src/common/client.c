#include "client.h"

mqtt_conn_t mqtt_conn;

gru_status_t smart_client_connect(const char *connect_url, const char *id) {
    logger_t logger = gru_logger_get();
    gru_status_t status = gru_status_new();

    int rc = MQTTClient_create(&mqtt_conn.client,
                               connect_url, id,
                               MQTTCLIENT_PERSISTENCE_NONE,
                               NULL);


    if (rc != MQTTCLIENT_SUCCESS) {
        gru_status_set(&status, GRU_FAILURE, "Unable to create MQTT client handle: %d", rc);

        return status;
    }

    logger(GRU_DEBUG, "Setting connection options");
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.MQTTVersion = MQTTVERSION_3_1_1;

    rc = MQTTClient_connect(mqtt_conn.client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        gru_status_set(&status, GRU_FAILURE, "Unable to connect: %d", rc);

        return status;
    }

    return status;
}

gru_status_t smart_client_subscribe(const char *topic) {
    gru_status_t status = gru_status_new();
    logger_t logger = gru_logger_get();

    logger(GRU_DEBUG, "Subscribing to %s", topic);
    int rc = MQTTClient_subscribe(mqtt_conn.client, topic, QOS_AT_MOST_ONCE);
    if (rc != MQTTCLIENT_SUCCESS) {
        gru_status_set(&status, GRU_FAILURE, "Unable to subscribe: %d", rc);

        return status;
    }

    return status;
}

gru_status_t smart_client_receive(smart_client_receive_fn receive_callback, smart_client_send_fn reply_callback) {
    char *received_topic;
    int received_topic_len;

    logger_t logger = gru_logger_get();
    gru_status_t status = gru_status_new();
    logger(GRU_DEBUG, "Waiting for messages");

    do {
        MQTTClient_message *msg = NULL;
        int rc = MQTTClient_receive(mqtt_conn.client, &received_topic, &received_topic_len, &msg, 10000);
        switch (rc) {
            case MQTTCLIENT_SUCCESS: {
                if (!msg) {
                    // No data received, so send a ping to prevent remote disconnect
                    MQTTClient_yield();
                    logger(GRU_INFO, "No data");
                }
                else {
                    reply_t *reply = receive_callback(received_topic, msg->payload, msg->payloadlen, &status);
                    if (!gru_status_success(&status)) {
                        logger(GRU_WARNING, "The callback function did not complete successfully");
                    }
                    else {
                        reply_callback(reply);
                    }
                }

                break;
            }
            case MQTTCLIENT_TOPICNAME_TRUNCATED: {
                logger(GRU_WARNING, "Topic name truncated");
                break;
            }
            default: {
                gru_status_set(&status, GRU_FAILURE, "Unable to receive data: error %d", rc);
                return status;
            }
        }
    } while (true);

    return status;
}

gru_status_t smart_client_send(reply_t *reply) {
    MQTTClient_deliveryToken token;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    gru_status_t status = gru_status_new();

    int rc = 0;

    pubmsg.payload = reply->payload;
    pubmsg.payloadlen = reply->len;

    if (unlikely(pubmsg.payloadlen > INT_MAX)) {
        gru_status_set(&status,
                       GRU_FAILURE,
                       "Data to big to serialize on MQTT protocol: max %d / size: %ld",
                       INT_MAX,
                       pubmsg.payloadlen);

        return status;
    }

    rc = MQTTClient_publishMessage(mqtt_conn.client, reply->topic, &pubmsg, &token);

    switch (rc) {
        case MQTTCLIENT_SUCCESS:
            break;
        default: {
            gru_status_set(&status, GRU_FAILURE, "Unable to publish the message: error %d", rc);

            return status;
        }
    }

    rc = MQTTClient_waitForCompletion(mqtt_conn.client, token, 1000);
    switch (rc) {
        case MQTTCLIENT_SUCCESS:
            break;
        default: {
            gru_status_set(&status, GRU_FAILURE, "Unable to synchronize: error %d", rc);

            return status;
        }
    }

    return status;
}
