#include <stdbool.h>

#include <libwebsockets.h>
#include <semaphore.h>
#include <pthread.h>

#include "common.h"
#include "server.h"
#include "tun.h"

#define C_READ_BUFFER_SIZE 1504

static struct lws_context *s_serverLwsContext = NULL;
static struct lws *s_clientWsi = NULL;
static pthread_t s_serverTunReadThread;
static sem_t s_clientSendSemaphore;
static uint8_t s_clientSendBuffer[LWS_PRE + C_READ_BUFFER_SIZE];
static size_t s_clientSendBufferSize;
static bool s_sendRequest = false;

struct t_serverInfo {
    struct lws_context *context;
    struct lws_vhost *vhost;
    const struct lws_protocols *protocol;
    struct lws_client_connect_info info;
    struct lws *wsi;
};

static int serverCreateTunReadThread(void);
static void *serverTunReadThreadMain(void *p_arg);

static int callback_vpn(
    struct lws *p_wsi,
    enum lws_callback_reasons p_reason,
	void *p_user,
    void *p_in,
    size_t p_len
);

static int callback_vpnInit(
    struct lws *p_wsi,
    enum lws_callback_reasons p_reason,
	void *p_user,
    void *p_in,
    size_t p_len
);

static int callback_vpnNewClient(
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

static int callback_vpnDisconnect(
    struct lws *p_wsi,
    enum lws_callback_reasons p_reason,
	void *p_user,
    void *p_in,
    size_t p_len
);

int serverInit(void) {
    const struct lws_protocols l_lwsProtocols[] = {
        { "vpn", callback_vpn, sizeof(struct t_serverInfo), 1504, 0, NULL, 1504 },
        { NULL, NULL, 0, 0, 0, NULL, 0 }
    };

    struct lws_context_creation_info l_lwsCreateContextInfo;
    memset(&l_lwsCreateContextInfo, 0, sizeof(l_lwsCreateContextInfo));
    l_lwsCreateContextInfo.port = 5228,
    l_lwsCreateContextInfo.protocols = l_lwsProtocols;

    s_serverLwsContext = lws_create_context(&l_lwsCreateContextInfo);

    sem_init(&s_clientSendSemaphore, 0, 1);

    return 0;
}

int serverStart(void) {
    return serverCreateTunReadThread();
}

int serverExecute(void) {
    return lws_service(s_serverLwsContext, 0);
}

int serverQuit(void) {
    lws_context_destroy(s_serverLwsContext);

    return 0;
}

void serverSend(const void *p_buffer, size_t p_packetSize) {
    if(s_clientWsi != NULL) {
        sem_wait(&s_clientSendSemaphore);

        memcpy(&s_clientSendBuffer[LWS_PRE], p_buffer, p_packetSize);
        s_clientSendBufferSize = p_packetSize;

        s_sendRequest = true;
        lws_cancel_service(s_serverLwsContext);
    }
}

static int serverCreateTunReadThread(void) {
    return pthread_create(
        &s_serverTunReadThread,
        NULL,
        serverTunReadThreadMain,
        NULL
    );
}

static void *serverTunReadThreadMain(void *p_arg) {
    M_UNUSED_PARAMETER(p_arg);

    while(true) {
        uint8_t l_buffer[C_READ_BUFFER_SIZE];

        ssize_t l_packetSize = tunRead(l_buffer);

        if(l_packetSize <= 0) {
            break;
        }

        serverSend(l_buffer, l_packetSize);
    }

    return NULL;
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
        case LWS_CALLBACK_PROTOCOL_INIT:
            l_returnValue = callback_vpnInit(
                p_wsi,
                p_reason,
                p_user,
                p_in,
                p_len
            );

            break;

        case LWS_CALLBACK_WSI_CREATE:
            l_returnValue = callback_vpnNewClient(
                p_wsi,
                p_reason,
                p_user,
                p_in,
                p_len
            );

            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            lws_write(p_wsi, &s_clientSendBuffer[LWS_PRE], s_clientSendBufferSize, LWS_WRITE_BINARY);
            sem_post(&s_clientSendSemaphore);
            break;

        case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
            if(s_clientWsi != NULL && s_sendRequest) {
                lws_callback_on_writable(s_clientWsi);
            }

            break;

        case LWS_CALLBACK_RECEIVE:
            l_returnValue = callback_vpnReceive(
                p_wsi,
                p_reason,
                p_user,
                p_in,
                p_len
            );

            break;

        case LWS_CALLBACK_CLOSED:
            l_returnValue = callback_vpnDisconnect(
                p_wsi,
                p_reason,
                p_user,
                p_in,
                p_len
            );

            break;

        default:
            break;
    }

    return l_returnValue;
}

static int callback_vpnInit(
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

    struct lws_vhost *l_vhost = lws_get_vhost(p_wsi);
    const struct lws_protocols *l_protocol = lws_get_protocol(p_wsi);

    struct t_serverInfo *l_serverInfo =
        lws_protocol_vh_priv_zalloc(
            l_vhost,
            l_protocol,
            sizeof(struct t_serverInfo)
        );

    l_serverInfo->context = lws_get_context(p_wsi);
    l_serverInfo->protocol = l_protocol;
    l_serverInfo->vhost = l_vhost;

    return 0;
}

static int callback_vpnNewClient(
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

    s_clientWsi = (struct lws *)p_wsi;

    sem_destroy(&s_clientSendSemaphore);
    sem_init(&s_clientSendSemaphore, 0, 1);

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

    tunWrite(p_in, p_len);

    return 0;
}

static int callback_vpnDisconnect(
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

    s_clientWsi = NULL;

    return 0;
}
