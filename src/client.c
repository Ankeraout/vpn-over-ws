#include <stdbool.h>

#include <libwebsockets.h>
#include <pthread.h>

#include "common.h"
#include "tun.h"

#define C_READ_BUFFER_SIZE 1504

struct t_clientInfo {
    struct lws_context *context;
    struct lws_vhost *vhost;
    const struct lws_protocols *protocol;
    struct lws_client_connect_info info;
    struct lws *wsi;
};

static int callback_vpn(
    struct lws *p_wsi,
    enum lws_callback_reasons p_reason,
	void *p_user,
    void *p_in,
    size_t p_len
);

static int callback_vpnWriteable(
    struct lws *p_wsi,
    enum lws_callback_reasons p_reason,
	void *p_user,
    void *p_in,
    size_t p_len
);

static int callback_vpnReceive(
    struct lws *p_wsi,
    enum lws_callback_reasons p_reason,
	void *p_user,
    void *p_in,
    size_t p_len
);

static int callback_vpnEstablished(
    struct lws *p_wsi,
    enum lws_callback_reasons p_reason,
	void *p_user,
    void *p_in,
    size_t p_len
);

static int clientCreateTunReadThread(void);
static void *clientTunReadThreadMain(void *p_arg);

static struct lws_context *s_clientLwsContext;
static struct t_clientInfo s_clientInfo;
static bool s_isConnectionEstablished = false;
static pthread_t s_clientTunReadThread;
static sem_t s_clientSendSemaphore;
static uint8_t s_clientSendBuffer[LWS_PRE + C_READ_BUFFER_SIZE];
static size_t s_clientSendBufferSize;
static bool s_sendRequest = false;

void clientSend(const void *p_buffer, size_t p_packetSize) {
    if(s_isConnectionEstablished) {
        sem_wait(&s_clientSendSemaphore);

        memcpy(&s_clientSendBuffer[LWS_PRE], p_buffer, p_packetSize);
        s_clientSendBufferSize = p_packetSize;

        s_sendRequest = true;
        lws_cancel_service(s_clientLwsContext);
    }
}

static int clientCreateTunReadThread(void) {
    return pthread_create(
        &s_clientTunReadThread,
        NULL,
        clientTunReadThreadMain,
        NULL
    );
}

static void *clientTunReadThreadMain(void *p_arg) {
    M_UNUSED_PARAMETER(p_arg);

    while(true) {
        uint8_t l_buffer[C_READ_BUFFER_SIZE];

        ssize_t l_packetSize = tunRead(l_buffer);

        if(l_packetSize <= 0) {
            break;
        }

        clientSend(l_buffer, l_packetSize);
    }

    return NULL;
}

int clientInit(const char *p_url) {
    // Initialize semaphores
    sem_init(&s_clientSendSemaphore, 0, 1);

    // Initialize LWS
    const struct lws_protocols l_lwsProtocols[] = {
        { "vpn", callback_vpn, sizeof(struct t_clientInfo), 1504, 0, NULL, 1504 },
        { NULL, NULL, 0, 0, 0, NULL, 0 }
    };

    struct lws_context_creation_info l_lwsCreateContextInfo;
    memset(&l_lwsCreateContextInfo, 0, sizeof(l_lwsCreateContextInfo));
    l_lwsCreateContextInfo.port = CONTEXT_PORT_NO_LISTEN,
    l_lwsCreateContextInfo.protocols = l_lwsProtocols;
    l_lwsCreateContextInfo.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

    s_clientLwsContext = lws_create_context(&l_lwsCreateContextInfo);

    // Parse the URL
    const char *l_protocol = NULL;
    const char *l_address = NULL;
    int l_port;
    const char *l_path = NULL;

    char l_url[strlen(p_url) + 1];
    strcpy(l_url, p_url);

    int l_returnValue = lws_parse_uri(
        l_url,
        &l_protocol,
        &l_address,
        &l_port,
        &l_path
    );

    char l_fixedPath[strlen(l_path) + 2];
    l_fixedPath[0] = '/';
    strcpy(&l_fixedPath[1], l_path);

    if(l_returnValue) {
        printf("Failed to parse URL.\n");
        return 1;
    }

    // Create client connect info
    struct lws_client_connect_info l_lwsClientConnectInfo;
    memset(&l_lwsClientConnectInfo, 0, sizeof(l_lwsClientConnectInfo));

    l_lwsClientConnectInfo.address = l_address;
    l_lwsClientConnectInfo.port = l_port;
    l_lwsClientConnectInfo.path = l_fixedPath;
    l_lwsClientConnectInfo.protocol = "vpn";
    l_lwsClientConnectInfo.context = s_clientLwsContext;
    l_lwsClientConnectInfo.host = l_address;
    l_lwsClientConnectInfo.origin = l_address;
    l_lwsClientConnectInfo.ietf_version_or_minus_one = -1;
    l_lwsClientConnectInfo.pwsi = &s_clientInfo.wsi;

    if(strcmp(l_protocol, "https") == 0) {
        l_lwsClientConnectInfo.ssl_connection = LCCSCF_USE_SSL;
    }

    lws_client_connect_via_info(&l_lwsClientConnectInfo);

    return 0;
}

int clientStart(void) {
    return clientCreateTunReadThread();
}

int clientExecute(void) {
    return lws_service(s_clientLwsContext, 0);
}

int clientQuit(void) {
    lws_context_destroy(s_clientLwsContext);

    return 0;
}

static int callback_vpn(
    struct lws *p_wsi,
    enum lws_callback_reasons p_reason,
	void *p_user,
    void *p_in,
    size_t p_len
) {
    int l_returnValue = 0;

    switch(p_reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            l_returnValue = callback_vpnEstablished(
                p_wsi,
                p_reason,
                p_user,
                p_in,
                p_len
            );

            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            l_returnValue = callback_vpnReceive(
                p_wsi,
                p_reason,
                p_user,
                p_in,
                p_len
            );

            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            l_returnValue = callback_vpnWriteable(
                p_wsi,
                p_reason,
                p_user,
                p_in,
                p_len
            );

            break;

        case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
            if(s_isConnectionEstablished && s_sendRequest) {
                lws_callback_on_writable(s_clientInfo.wsi);
            }

            break;

        default:
            break;
    }

    return l_returnValue;
}

static int callback_vpnEstablished(
    struct lws *p_wsi,
    enum lws_callback_reasons p_reason,
	void *p_user,
    void *p_in,
    size_t p_len
) {
    M_UNUSED_PARAMETER(p_wsi);
    M_UNUSED_PARAMETER(p_reason);
    M_UNUSED_PARAMETER(p_user);
    M_UNUSED_PARAMETER(p_in);
    M_UNUSED_PARAMETER(p_len);

    s_isConnectionEstablished = true;

    return 0;
}

static int callback_vpnWriteable(
    struct lws *p_wsi,
    enum lws_callback_reasons p_reason,
	void *p_user,
    void *p_in,
    size_t p_len
) {
    M_UNUSED_PARAMETER(p_reason);
    M_UNUSED_PARAMETER(p_user);
    M_UNUSED_PARAMETER(p_in);
    M_UNUSED_PARAMETER(p_len);

    lws_write(p_wsi, &s_clientSendBuffer[LWS_PRE], s_clientSendBufferSize, LWS_WRITE_BINARY);
    sem_post(&s_clientSendSemaphore);

    return 0;
}

static int callback_vpnReceive(
    struct lws *p_wsi,
    enum lws_callback_reasons p_reason,
	void *p_user,
    void *p_in,
    size_t p_len
) {
    M_UNUSED_PARAMETER(p_wsi);
    M_UNUSED_PARAMETER(p_reason);
    M_UNUSED_PARAMETER(p_user);

    return tunWrite(p_in, p_len);
}
