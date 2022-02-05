#include <libwebsockets.h>

#include "common.h"

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

static struct lws_context *s_clientLwsContext;

static struct t_clientInfo s_clientInfo;

int clientInit(const char *p_url) {
    // Initialize LWS
    const struct lws_protocols l_lwsProtocols[] = {
        { "vpn", callback_vpn, sizeof(struct t_clientInfo), 1504, 0, NULL, 0 },
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

    if(l_returnValue) {
        printf("Failed to parse URL.\n");
        return 1;
    } else {
        printf(
            "URL: %s\nProtocol: %s\nAddress: %s\nPort: %d\nPath: %s\n",
            p_url,
            l_protocol,
            l_address,
            l_port,
            l_path
        );
    }

    // Create client connect info
    struct lws_client_connect_info l_lwsClientConnectInfo;
    memset(&l_lwsClientConnectInfo, 0, sizeof(l_lwsClientConnectInfo));

    l_lwsClientConnectInfo.address = l_address;
    l_lwsClientConnectInfo.port = l_port;
    l_lwsClientConnectInfo.path = "/vpn/";
    l_lwsClientConnectInfo.protocol = "vpn";
    l_lwsClientConnectInfo.context = s_clientLwsContext;
    l_lwsClientConnectInfo.host = l_address;
    l_lwsClientConnectInfo.origin = l_address;
    l_lwsClientConnectInfo.ietf_version_or_minus_one = -1;
    l_lwsClientConnectInfo.pwsi = &s_clientInfo.wsi;
    l_lwsClientConnectInfo.ssl_connection = LCCSCF_USE_SSL;

    lws_client_connect_via_info(&l_lwsClientConnectInfo);

    return 0;
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
    M_UNUSED_PARAMETER(p_wsi);
    M_UNUSED_PARAMETER(p_reason);
    M_UNUSED_PARAMETER(p_user);
    M_UNUSED_PARAMETER(p_in);
    M_UNUSED_PARAMETER(p_len);

    int l_returnValue = 0;

    printf("callback_vpn() called. reason=%d\n", p_reason);

    switch(p_reason) {
        default:
            break;
    }

    return l_returnValue;
}
