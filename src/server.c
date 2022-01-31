#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>

#include "common.h"
#include "tun.h"

static void *serverListenThreadMain(void *arg);
static void *clientReceiveThreadMain(void *arg);

int serverSocket;
pthread_t serverListenThread;
int clientSocket = 0;

int serverStart(void) {
    if(tunInit("192.168.128.1")) {
        fprintf(stderr, "Failed to create tun device.\n");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket < 0) {
        perror("Failed to create the server socket");
        return 1;
    }

    struct sockaddr_in serverSocketAddress = {
        .sin_addr.s_addr = INADDR_ANY,
        .sin_family = AF_INET,
        .sin_port = htons(5228)
    };

    memset(
        serverSocketAddress.sin_zero,
        0,
        sizeof(serverSocketAddress.sin_zero)
    );

    if(
        bind(
            serverSocket,
            (struct sockaddr *)&serverSocketAddress,
            sizeof(serverSocketAddress)
        ) != 0
    ) {
        perror("Failed to bind socket");
        close(serverSocket);
        return 1;
    }

    if(listen(serverSocket, 1) != 0) {
        perror("Failed to put the server socket in listening state");
        close(serverSocket);
        return 1;
    }

    if(
        pthread_create(
            &serverListenThread,
            NULL,
            serverListenThreadMain,
            NULL
        ) != 0
    ) {
        perror("Failed to create listener thread");
        close(serverSocket);
        return 1;
    }

    printf("Server started.\n");

    return 0;
}

static void *serverListenThreadMain(void *arg) {
    UNUSED_PARAMETER(arg);

    int acceptedClient;

    do {
        struct sockaddr clientSocketAddress;
        socklen_t clientSocketAddressLength = sizeof(clientSocketAddress);

        acceptedClient = accept(
            serverSocket,
            &clientSocketAddress,
            &clientSocketAddressLength
        );

        if(acceptedClient >= 0) {
            pthread_t clientThread;

            if(
                pthread_create(
                    &clientThread,
                    NULL,
                    clientReceiveThreadMain,
                    (void *)((size_t)acceptedClient)
                ) != 0
            ) {
                perror("Failed to create client socket");
                close(acceptedClient);
            }
        }
    } while(acceptedClient >= 0);

    return NULL;
}

static void *clientReceiveThreadMain(void *arg) {
    UNUSED_PARAMETER(arg);

    printf("Accepted incoming client connection.\n");

    clientSocket = (int)((size_t)arg);

    // Ignore the HTTP request header
    bool flag_cr = false;
    int crlfCount = 0;
    char buffer;

    printf("clientSocket: %d\n", clientSocket);

    int readResult = read(clientSocket, &buffer, 1);

    do {   
        printf("Read\n");
        putc(buffer, stdout);

        if(flag_cr) {
            if(buffer == '\n') {
                crlfCount++;
            } else {
                flag_cr = false;
                crlfCount = 0;
            }
        } else if(buffer == '\r') {
            flag_cr = true;
        } else {
            crlfCount = 0;
        }

        readResult = read(clientSocket, &buffer, 1);
    } while((readResult > 0) && (crlfCount < 2));

    if(crlfCount != 2) {
        printf("Failed to parse HTTP request, disconnecting client.\n");
        close(clientSocket);
        return NULL;
    }

    printf("Passed HTTP header\n");

    while(true) {
        uint16_t packetLength;

        ssize_t returnValue = readForce(clientSocket, &packetLength, sizeof(packetLength));

        printf("Read\n");

        if(returnValue <= 0) {
            break;
        }

        uint8_t buffer[packetLength];

        returnValue = readForce(clientSocket, &buffer, packetLength);

        if(returnValue <= 0) {
            break;
        }

        tunWrite(buffer, returnValue);
    }

    return NULL;
}

int serverWrite(const void *buffer, size_t bufferSize) {
    printf("serverWrite() called\n");
    if(clientSocket != 0) {
        printf("Sending\n");
        write(clientSocket, buffer, bufferSize);
    }

    return 0;
}
