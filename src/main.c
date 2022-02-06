#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "server.h"
#include "tun.h"

#define C_IP_TUN_SERVER "192.168.128.1"
#define C_IP_TUN_CLIENT "192.168.128.2"

static const char *s_url;

int parseCommandLineParameters(int p_argc, const char *p_argv[]);

int main(int p_argc, const char *p_argv[]) {
    if(parseCommandLineParameters(p_argc, p_argv)) {
        return EXIT_FAILURE;
    }

    if(s_url == NULL) {
        if(serverInit()) {
            return EXIT_FAILURE;
        }

        if(tunInit(C_IP_TUN_SERVER)) {
            return EXIT_FAILURE;
        }

        if(serverStart()) {
            return EXIT_FAILURE;
        }

        while(true) {
            serverExecute();
        }

        serverQuit();
    } else {
        if(clientInit(s_url)) {
            return EXIT_FAILURE;
        }

        if(tunInit(C_IP_TUN_CLIENT)) {
            return EXIT_FAILURE;
        }

        if(clientStart()) {
            return EXIT_FAILURE;
        }

        while(true) {
            clientExecute();
        }

        clientQuit();
    }

    return EXIT_SUCCESS;
}

int parseCommandLineParameters(int p_argc, const char *p_argv[]) {
    s_url = NULL;

    bool l_flagError = false;

    for(int l_index = 1; l_index < p_argc; l_index++) {
        const char *l_arg = p_argv[l_index];

        if(s_url == NULL) {
            s_url = l_arg;
        } else {
            fprintf(stderr, "Error: too many URLs.\n");
            l_flagError = true;
        }
    }

    if(l_flagError) {
        return 1;
    } else {
        return 0;
    }
}
