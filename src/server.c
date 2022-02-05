#include <stdbool.h>

#include <libwebsockets.h>

#include "common.h"
#include "server.h"

static struct lws_context *s_serverLwsContext;

struct t_serverInfo {
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
        { "vpn", callback_vpn, sizeof(struct t_serverInfo), 1504, 0, NULL, 0 },
        { NULL, NULL, 0, 0, 0, NULL, 0 }
    };

    struct lws_context_creation_info l_lwsCreateContextInfo;
    memset(&l_lwsCreateContextInfo, 0, sizeof(l_lwsCreateContextInfo));
    l_lwsCreateContextInfo.port = 5228,
    l_lwsCreateContextInfo.protocols = l_lwsProtocols;

    s_serverLwsContext = lws_create_context(&l_lwsCreateContextInfo);

    return 0;
}

int serverExecute(void) {
    return lws_service(s_serverLwsContext, 0);
}

int serverQuit(void) {
    lws_context_destroy(s_serverLwsContext);

    return 0;
}

static int callback_vpn(
    struct lws *p_wsi,
    enum lws_callback_reasons p_reason,
	void *p_user,
    void *p_in,
    size_t p_len
) {
    printf("callback_vpn() called. reason=%d\n", p_reason);

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

        case LWS_CALLBACK_ESTABLISHED:
            l_returnValue = callback_vpnNewClient(
                p_wsi,
                p_reason,
                p_user,
                p_in,
                p_len
            );

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

    printf("callback_vpnInit()\n");

    return 0;
}

static int callback_vpnNewClient(
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

    printf("callback_vpnNewClient()\n");

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

    char l_buffer[p_len + 1];

    memcpy(l_buffer, p_in, p_len);
    l_buffer[p_len] = '\0';

    printf("Received data: %s\n", l_buffer);

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

    printf("Client disconnected\n");

    return 0;
}
