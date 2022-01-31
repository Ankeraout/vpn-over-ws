#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "server.h"
#include "tun.h"

bool flag_server = false;
bool flag_tap = false;
const char *url = NULL;

static int parseCommandLineParameters(int argc, const char *argv[]);

int main(int argc, const char *argv[]) {
    if(parseCommandLineParameters(argc, argv) != 0) {
        return EXIT_FAILURE;
    }

    if(flag_server) {
        if(serverStart()) {
            return EXIT_FAILURE;
        }
    } else {
        if(clientConnect(url) != 0) {
            return EXIT_FAILURE;
        }
    }

    tunLoop();

    return EXIT_SUCCESS;
}

static int parseCommandLineParameters(int argc, const char *argv[]) {
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--server") == 0) {
            flag_server = true;
        } else if(strcmp(argv[i], "--tap") == 0) {
            flag_tap = true;
        } else {
            url = argv[i];
        }
    }

    if((!flag_server) && (url == NULL)) {
        fprintf(stderr, "Arguments parsing failed.\n");
        return 1;
    }

    return 0;
}
