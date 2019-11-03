#include "main.h"

typedef struct options_t_ {
    char *uri;
    log_level_t log_level;
    char *id;
    bool daemon;
    bool stateless;
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
    gru_cli_option_help("log-level", "l", "runs in the given level (info, stat, debug, etc) mode");
    gru_cli_option_help("stateless", "s", "do not update current state to the broker");
    gru_cli_option_help("id", "i", "client id");
}

static reply_t *power_control_callback(const char *topic, const void *payload, int len, gru_status_t *status) {
    logger_t logger = gru_logger_get();
    reply_t *reply = NULL;

    logger(GRU_INFO, "Received %d bytes of data on the topic %s: %s", len, topic, payload);

    const char *app_home = gru_base_app_home("smart-pc-control");
    char *command;

    if (asprintf(&command, "%s/%s/script.sh %s", app_home, topic, payload) == -1) {
        gru_status_set(status, GRU_FAILURE, "Not enough memory to execute script");

        return NULL;
    }
    logger(GRU_INFO, "About to execute %s with args %s", command, payload);
    int sret = system(command);
    switch (sret) {
        case 0: {
            logger(GRU_INFO, "Executed successfully");

            reply = gru_alloc(sizeof(reply_t), status);
            gru_alloc_check(reply, NULL);

            if (!options.stateless) {
                reply->payload = strdup((char *) payload);
                reply->len = len;
                reply->topic = strdup("pc/nuc/status/on");
            }

            break;
        }
        case -1: {
            logger(GRU_ERROR, "The child process could not be created");
            break;
        }
        case 127: {
            logger(GRU_ERROR, "The shell process could not be executed");
            break;
        }
        default: {
            logger(GRU_ERROR, "The script may have failed");
            break;
        }
    }

    gru_dealloc_string(&command);

    return reply;
}

static int run() {
    logger_t logger = gru_logger_get();
    gru_status_t status = gru_status_new();

    const char *app_home = gru_base_app_home("smart-pc-control");
    if (!gru_path_mkdirs(app_home, &status)) {
        logger(GRU_FATAL, "Failed to create application directories: %s", status.message);
        return EXIT_FAILURE;
    }

    int retry = 10;
    while (retry > 0) {
        status = smart_client_connect(options.uri, options.id);
        if (!gru_status_success(&status)) {
            logger(GRU_FATAL, "Failed to connect to MQTT broker: %s", status.message);
            retry--;

            if (retry == 0) {
                return EXIT_FAILURE;
            }
            sleep(5);
        }
        else {
            break;
        }
    }



    status = smart_client_subscribe("pc/nuc/state/on");
    if (!gru_status_success(&status)) {
        logger(GRU_FATAL, "Failed to subscribe to the topic: %s", status.message);
        return EXIT_FAILURE;
    }

    reply_t reply = {
            .topic = "pc/nuc/status/on",
            .payload = "true",
            .len = strlen("true")
    };

    if (options.stateless) {
        logger(GRU_DEBUG, "Running stateless, therefore not resetting the state");
    }
    else {
        status = smart_client_send(&reply);
        if (!gru_status_success(&status)) {
            logger(GRU_FATAL, "Failed to reset PC state: %s", status.message);
        }
    }

    status = smart_client_receive(power_control_callback, smart_client_send);
    if (!gru_status_success(&status)) {
        logger(GRU_FATAL, "Failed to receive data from MQTT broker: %s", status.message);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    int option_index = 0;

    if (argc < 2) {
        show_help(argv);

        return EXIT_FAILURE;
    }

    gru_logger_set(gru_logger_default_printer);
    gru_logger_set_minimum(GRU_INFO);

    options.daemon = false;
    while (1) {

        static struct option long_options[] = {
                {"broker-url",     required_argument, 0, 'b'},
                {"log-level",      required_argument, 0, 'l'},
                {"id",             required_argument, 0, 'i'},
                {"daemon",         no_argument,       0, 'd'},
                {"stateless",      no_argument,       0, 's'},
                {0,                0,                 0, 0}
        };

        int c = getopt_long(
                argc, argv, "b:l:i:ds", long_options, &option_index);
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
            case 'i':
                options.id = optarg;
                break;
            case 'd':
                options.daemon = true;
                break;
            case 's':
                options.stateless = true;
                break;
            case 'h':
                show_help(argv);
//                options_destroy(&options);

                return EXIT_SUCCESS;
            default:
                printf("Invalid or missing option\n");
                show_help(argv);

                return EXIT_FAILURE;
        }
    }

    if (options.daemon) {
        pid_t child = fork();

        if (child == 0) {
            return run();
        }
    }
    else {
        return run();
    }
}