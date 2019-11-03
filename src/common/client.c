#include "client.h"

mqtt_conn_t mqtt_conn;

gru_status_t smart_client_connect(char *connect_url) {
    logger_t logger = gru_logger_get();
    gru_status_t status = gru_status_new();

    int rc = MQTTClient_create(&mqtt_conn.client,
                               connect_url, "smart-pc-controller",
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

gru_status_t smart_client_receive() {
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
                    logger(GRU_INFO, "Received data on the topic %s: %s", received_topic, msg->payload);
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

