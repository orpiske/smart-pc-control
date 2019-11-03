#include "main.h"

typedef struct options_t_ {
    char *uri;
    log_level_t log_level;
} options_t;

static options_t options;

void options_set_log_level(const char *value) {
    options.log_level = gru_logger_get_level(value);

    gru_logger_set_minimum(options.log_level);
}

static void show_help(char **argv) {
    gru_cli_program_usage("smart-power-daemon", argv[0]);

    gru_cli_option_help("help", "h", "show this help");

    gru_cli_option_help("broker-url", "b", "broker URL to connect to");
    gru_cli_option_help("log-level",
                        "l",
                        "runs in the given verbose (info, stat, debug, etc) level mode");
}

static gru_status_t power_control_callback(const char *topic, const void *payload, int len) {
    logger_t logger = gru_logger_get();
    gru_status_t status = gru_status_new();

    logger(GRU_INFO, "Received %d bytes of data on the topic %s: %s", len, topic, payload);
    return status;
}

int main(int argc, char **argv) {
    int option_index = 0;

    if (argc < 2) {
        show_help(argv);

        return EXIT_FAILURE;
    }

    gru_logger_set(gru_logger_default_printer);
    gru_logger_set_minimum(GRU_INFO);

    while (1) {

        static struct option long_options[] = {
                {"broker-url",     required_argument, 0, 'b'},
                {"log-level",      required_argument, 0, 'l'},
                {0,                0,                 0, 0}
        };

        int c = getopt_long(
                argc, argv, "b:l:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'b':
                options.uri = optarg;
                break;
            case 'l':
                options_set_log_level(optarg);
                break;
            default:
                printf("Invalid or missing option\n");
                show_help(argv);

                return EXIT_FAILURE;
        }
    }

    gru_status_t status = smart_client_connect(options.uri);
    if (!gru_status_success(&status)) {
        logger_t logger = gru_logger_get();

        logger(GRU_FATAL, "Failed to connect to MQTT broker: %s", status.message);
        return EXIT_FAILURE;
    }

    status = smart_client_subscribe("pc/nuc/state/on");
    if (!gru_status_success(&status)) {
        logger_t logger = gru_logger_get();

        logger(GRU_FATAL, "Failed to subscribe to the topic: %s", status.message);
        return EXIT_FAILURE;
    }

    status = smart_client_receive(power_control_callback);
    if (!gru_status_success(&status)) {
        logger_t logger = gru_logger_get();

        logger(GRU_FATAL, "Failed to receive data from MQTT broker: %s", status.message);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}