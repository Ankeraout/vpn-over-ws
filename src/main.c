#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "server.h"

static const char *s_url;

int parseCommandLineParameters(int p_argc, const char *p_argv[]);

int main(int p_argc, const char *p_argv[]) {
    if(parseCommandLineParameters(p_argc, p_argv)) {
        return EXIT_FAILURE;
    }

    if(s_url == NULL) {
        serverInit();

        while(true) {
            serverExecute();
        }

        serverQuit();
    } else {
        clientInit(s_url);

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

        printf("Arg: %s\n", l_arg);

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
