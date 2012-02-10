#include "net.h"
#include "net/iocp.h"
#include "io/parsers/iniparser.h"

#define CONFIG_FILE "CONFIG/net.ini"

static net_func cb = net_onnotify;

int
net_init(void)
{
    int res = 0;
    WSADATA data;

    MODULE_IMPORT_SWITCH;
    public_init();

    if (WSAStartup(MAKEWORD(2, 0), &data) != 0) {
        LOG_ERROR("pub.net", "WSAStartup failed: %u.",
                  GetLastError());
        res = -1;
    } else {
        net_entity_init();
        net_load();
    }
    return res;
}

int
net_fini(void)
{
    MODULE_IMPORT_SWITCH;
    net_entity_fini();
    WSACleanup();
    public_fini();
    return 0;
}

int
net_load(void)
{
    size_t i, count;
    string name;

    CIniFile f(CONFIG_FILE);
    f.ReadFile();
    count = f.NumKeys();
    for (i = 0; i < count; ++i) {
        name = f.GetKeyName(i);
        net_entity_create(f.GetValueI(name, "id"),
                          f.GetValueI(name, "port"),
                          f.GetValue(name, "ip").c_str());
    }
    return 0;
}

void
net_register(net_func func)
{
    cb = func;
}

int
net_callback(SOCKET s, int type, int res, void *args)
{
    return cb(s, type, res, args);
}

int
net_onnotify(SOCKET s, int type, int res, void *args)
{
    SOCKADDR_IN addr;
    int addr_len = sizeof(addr);
    getsockname(s, (struct sockaddr *)&addr, &addr_len);

    switch (type) {
    case NIT_LSTN:
        getsockname(s, (struct sockaddr *)&addr, &addr_len);
        LOG_TRACE("pub.net", "%s:%u(%d) started.",
                  inet_ntoa(addr.sin_addr),
                  ntohs(addr.sin_port),
                  s);
        break;
    case NIT_CNCT:
        getsockname(s, (struct sockaddr *)&addr, &addr_len);
        if (res == 0) {
            LOG_TRACE("pub.net", "%s:%u(%d) connected.",
                      inet_ntoa(addr.sin_addr),
                      ntohs(addr.sin_port),
                      s);
        } else {
            if (res != WSAECONNREFUSED) {
                LOG_INFO("pub.net", "%s:%u(%d) connected failed: %d %s.",
                         inet_ntoa(addr.sin_addr),
                         ntohs(addr.sin_port),
                         s,
                         res,
                         net_errstr(res));
            }
            net_restart(*(int *)args);
        }
        break;
    case NIT_ACPT:
        getpeername(s, (struct sockaddr *)&addr, &addr_len);
        LOG_TRACE("pub.net", "%s:%u(%d) accepted.",
                  inet_ntoa(addr.sin_addr),
                  ntohs(addr.sin_port),
                  s);
        break;
    case NIT_SEND:
        if (res != 0) {
            getpeername(s, (struct sockaddr *)&addr, &addr_len);
            LOG_WARN("pub.net", "%s:%u(%d) send error: %s.",
                     inet_ntoa(addr.sin_addr),
                     ntohs(addr.sin_port),
                     s,
                     net_errstr(res));
            net_message_destroy((struct message_t *)args);
        }
        break;
    case NIT_SNDF:
        break;
    case NIT_RECV:
        break;
    case NIT_RCVF:
        net_message_destroy((struct message_t *)args);
        break;
    case NIT_ACLS:
        LOG_TRACE("pub.net", "closed actively.");
        break;
    case NIT_PCLS:
        LOG_TRACE("pub.net", "closed passively.");
        if (*(int *)args & NET_CLIENT_FLAG) {
            net_restart(*(int *)args);
        }
        break;
    default:
        LOG_ERROR("pub.net", "UNKNOWN NOTIFY TYPE: %d.", type);
        break;
    }
    return res;
}
