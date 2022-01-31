#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pthread.h>

#include "common.h"
#include "tun.h"

static int initSocket(const char *url);
static int initSSL(void);
static int sendRequest(const char *url);
static int startTun(void);
static int startThread(void);
static void *clientThreadMain(void *arg);

bool clientConnected = false;
int clientSocketClient;
SSL_CTX *sslContext;
SSL *ssl;

int clientConnect(const char *url) {
    // Initialize socket
    if(initSocket(url) != 0) {
        fprintf(stderr, "Failed to initialize socket.\n");
        return 1;
    }

    // Initialize OpenSSL
    if(initSSL() != 0) {
        fprintf(stderr, "Failed to initialize OpenSSL.\n");
        return 2;
    }

    // Send HTTP request
    if(sendRequest(url) != 0) {
        fprintf(stderr, "Failed to send HTTP request.\n");
        return 3;
    }

    // Start tun
    if(startTun() != 0) {
        fprintf(stderr, "Failed to send HTTP request.\n");
        return 4;
    }

    // Start client thread
    if(startThread() != 0) {
        fprintf(stderr, "Failed to start client thread.\n");
        return 5;
    }

    return 0;
}

static int initSocket(const char *url) {
    char hostname[512];

    if(sscanf(url, "https://%[^\\/]", hostname) == EOF) {
        return 1;
    }

    printf("URL: %s\nHostname: %s\n", url, hostname);

    struct hostent *host = gethostbyname(hostname);

    if(host == NULL) {
        perror(hostname);
        return 2;
    }

    struct sockaddr_in clientSocketAddress;

    clientSocketClient = socket(AF_INET, SOCK_STREAM, 0);
    memset(&clientSocketAddress, 0, sizeof(clientSocketAddress));
    clientSocketAddress.sin_family = AF_INET;
    clientSocketAddress.sin_port = htons(443);
    clientSocketAddress.sin_addr.s_addr = *(long *)(host->h_addr);

    int returnValue = connect(
        clientSocketClient,
        (struct sockaddr *)&clientSocketAddress,
        sizeof(clientSocketAddress)
    );

    if(returnValue != 0) {
        close(clientSocketClient);
        perror("Connection failed");
        return 3;
    }

    return 0;
}

static int initSSL(void) {
    SSL_library_init();

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    const SSL_METHOD *method = TLS_client_method();
    sslContext = SSL_CTX_new(method);

    if(sslContext == NULL) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    ssl = SSL_new(sslContext);
    SSL_set_fd(ssl, clientSocketClient);

    if(SSL_connect(ssl) == -1) {
        ERR_print_errors_fp(stderr);
        return 2;
    }

    return 0;
}

static int sendRequest(const char *url) {
    char hostname[512];
    char path[512];

    if(sscanf(url, "https://%[^\\/]%s", hostname, path) == EOF) {
        return 1;
    }

    printf("URL: %s\nHostname: %s\nPath: %s\n", url, hostname, path);

    char requestHeader[2048];

    sprintf(
        requestHeader,
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: http_vpn alpha 0.1\r\n"
        "\r\n",
        path,
        hostname
    );

    SSL_write(ssl, requestHeader, strlen(requestHeader));

    return 0;
}

static int startTun(void) {
    return tunInit("192.168.128.2");
}

static int startThread(void) {
    pthread_t thread;

    return pthread_create(
        &thread,
        NULL,
        clientThreadMain,
        NULL
    );
}

static void *clientThreadMain(void *arg) {
    UNUSED_PARAMETER(arg);

    clientConnected = true;

    while(true) {
        uint16_t packetSize;

        if(readForceSsl(ssl, &packetSize, 2) <= 0) {
            break;
        }

        printf("Read\n");

        uint8_t buffer[packetSize];

        if(readForceSsl(ssl, buffer, packetSize) <= 0) {
            break;
        }

        tunWrite(buffer, packetSize);
    }

    clientConnected = false;

    return NULL;
}

int clientWrite(const void *buffer, size_t bufferSize) {
    if(clientConnected) {
        SSL_write(ssl, buffer, bufferSize);
    }

    return 0;
}
