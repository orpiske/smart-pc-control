#include "main.h"

typedef struct options_t_ {
    char *uri;
    log_level_t log_level;
    char *id;
    bool daemon;
    int interval;
} options_t;

static options_t options;

static void options_set_log_level(const char *value) {
    options.log_level = gru_logger_get_level(value);

    gru_logger_set_minimum(options.log_level);
}

static void show_help(char **argv) {
    gru_cli_program_usage("smart-power-daemon", argv[0]);

    gru_cli_option_help("help", "h", "show this help");

    gru_cli_option_help("broker-url", "b", "broker URL to connect to");
    gru_cli_option_help("log-level", "l", "runs in the given level (info, stat, debug, etc) mode");
    gru_cli_option_help("update", "u", "update interval (defaults to 30 seconds)");
    gru_cli_option_help("id", "i", "client id");
}

static const char *readPayload(const char *dir, gru_status_t *status) {
    FILE *sensorDataFile = gru_io_open_file_read(dir, "out.script", status);
    if (sensorDataFile == NULL) {
        return NULL;
    }

    char *ret;
    size_t bytesRead = gru_io_read_text_into(&ret, sensorDataFile, status);
    if (bytesRead == 0) {
        fclose(sensorDataFile);
        return NULL;
    }

    fclose(sensorDataFile);
    return ret;
}

static const char *readSensor(const char *name, gru_status_t *status) {
    const char *temperatureData = NULL;
    logger_t logger = gru_logger_get();
    const char *app_home = gru_base_app_home("smart-pc-control");

    char *dir;
    if (asprintf(&dir, "%s/temperature-sensor/%s/", app_home, name) == -1) {
        gru_status_set(status, GRU_FAILURE, "Not enough memory to execute script");

        return NULL;
    }

    char *command;
    if (asprintf(&command, "%s/temperature-sensor/script.sh --%s --out %s", app_home, name, dir) == -1) {
        gru_status_set(status, GRU_FAILURE, "Not enough memory to execute script");

        goto err1;
    }

    switch (system(command)) {
        case 0: {
            logger(GRU_INFO, "Executed successfully");
            temperatureData = readPayload(dir, status);
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

    err1:
    gru_dealloc_string(&dir);
    return temperatureData;
}

static gru_status_t setActive(const char *baseTopic) {
    logger_t logger = gru_logger_get();
    gru_status_t status = gru_status_new();

    reply_t reply = {
            .payload = "true",
            .len = strlen(reply.payload)
    };

    if (asprintf(&reply.topic, "%s/active", baseTopic) == -1) {
        gru_status_set(&status, GRU_FAILURE, "Not enough memory to set the active status");

        return status;
    }

    status = smart_client_send(&reply);
    if (!gru_status_success(&status)) {
        logger(GRU_FATAL, "Send sensor data failed: %s", status.message);

        gru_dealloc_string(&reply.topic);
        return status;
    }

    return status;
}

static gru_status_t readTemperature(const char *name, const char *topic) {
    gru_status_t status = gru_status_new();
    logger_t logger = gru_logger_get();

    const char *sensorData = readSensor(name, &status);
    if (!sensorData) {
        return status;
    }

    reply_t reply = {
            .topic = (char *) topic,
            .payload = (char *) sensorData,
            .len = strlen(sensorData)
    };

    status = smart_client_send(&reply);
    if (!gru_status_success(&status)) {
        logger(GRU_FATAL, "Send sensor data failed: %s", status.message);

        gru_dealloc_const_string(&sensorData);
        return status;
    }

    status = setActive(topic);
    if (!gru_status_success(&status)) {
        logger(GRU_FATAL, "Set active state failed: %s", status.message);

        gru_dealloc_const_string(&sensorData);
        return status;
    }


    gru_dealloc_const_string(&sensorData);
    return status;
}

static int run() {
    logger_t logger = gru_logger_get();

    gru_status_t status = smart_client_init(options.uri, options.id);
    if (!gru_status_success(&status)) {
        logger(GRU_FATAL, "%s", status.message);
        return EXIT_FAILURE;
    }

    while(true) {
        status = readTemperature("package", "pc/nuc/temperature/cpu");
        if (!gru_status_success(&status)) {
            logger(GRU_ERROR, "Failed to read package temperature: %s", status.message);
        }

        status = readTemperature("acpi-1", "pc/nuc/temperature/acpi-1");
        if (!gru_status_success(&status)) {
            logger(GRU_ERROR, "Failed to read package temperature: %s", status.message);
        }

        status = readTemperature("acpi-2", "pc/nuc/temperature/acpi-2");
        if (!gru_status_success(&status)) {
            logger(GRU_ERROR, "Failed to read package temperature: %s", status.message);
        }

        sleep(options.interval);
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
    options.interval = 30;
    while (1) {

        static struct option long_options[] = {
                {"broker-url",     required_argument, 0, 'b'},
                {"log-level",      required_argument, 0, 'l'},
                {"id",             required_argument, 0, 'i'},
                {"daemon",         no_argument,       0, 'd'},
                {"stateless",      no_argument,       0, 's'},
                {"update",         no_argument,       0, 's'},
                {0,                0,                 0, 0}
        };

        int c = getopt_long(
                argc, argv, "b:l:i:u:ds", long_options, &option_index);
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
            case 'u':
                options.interval = atoi(optarg);
                break;
            case 'h':
                show_help(argv);

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