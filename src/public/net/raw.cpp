#include "raw.h"
#include "error.h"

#include "public/net.h"
#include "public/log/log.h"

int
net_socket(SOCKET *s)
{
    int res = 0;

    if (*s == INVALID_SOCKET) {
        *s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (*s == INVALID_SOCKET) {
            int err = WSAGetLastError();
            LOG_ERROR("pub.net", "WSASocket failed: %d %s.",
                err, net_errstr(err));
            res = NET_SOCKETINVALID;
        }
    }
    return res;
}

int
net_listen(SOCKET s, SOCKADDR_IN *addr)
{
    int res = 0;

    assert(addr);
    if (bind(s, (struct sockaddr *)addr, sizeof(*addr)) == SOCKET_ERROR ||
        listen(s, SOMAXCONN) == SOCKET_ERROR) {
            int err = WSAGetLastError();
            LOG_ERROR("pub.net", "listen failed: %d %s.",
                err, net_errstr(err));
            res = NET_SOCKETNOTBOUND;
    }
    return res;
}

int net_connect(SOCKET s, int id, SOCKADDR_IN *addr)
{
    int res = 0;

    assert(addr);
    if (connect(s, (struct sockaddr *)addr, sizeof(*addr)) == SOCKET_ERROR) {
        res = WSAGetLastError();
    }
    net_callback(s, NIT_CNCT, res, &id);
    return res;
}

int
net_shutdown(SOCKET s)
{
    int res = 0;

    if (shutdown(s, SD_BOTH) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAENOTCONN) {
            LOG_ERROR("pub.net", "shutdown failed: %d %s.",
                err, net_errstr(err));
            res = NET_SOCKETNOTSHUTDOWN;
        }
    }
    return res;
}

int
net_close(SOCKET *s)
{
    int res = 0;

    if (*s != INVALID_SOCKET) {
        if (closesocket(*s) == SOCKET_ERROR) {
            int err = WSAGetLastError();
            LOG_ERROR("pub.net", "closesocket failed: %d %s.",
                err, net_errstr(err));
            res = NET_SOCKETNOTCLOSED;
        } else {
            net_callback(*s, NIT_ACLS, 0, NULL);
            *s = INVALID_SOCKET;
        }
    }
    return res;
}

const char *
net_errstr(int err)
{
    const char *str;

    switch (err) {
    case WSAEINTR:
        str = "[WSAEINTR] The blocking call was canceled";
        break;
    case WSAEFAULT:
        str = "[WSAEFAULT] Invalid pointer address";
        break;
    case WSAEINVAL:
        str = "[WSAEINVAL] The socket has not been bound with bind.";
    case WSAEWOULDBLOCK:
        str = "[WSAEWOULDBLOCK] The socket is marked as nonblocking";
        break;
    case WSAEINPROGRESS:
        str = "[WSAEINPROGRESS] A blocking operation is executing";
        break;
    case WSAENOTSOCK:
        str = "[WSAENOTSOCK] The descriptor is not a socket";
        break;
    case WSAEMSGSIZE:
        str = "[WSAENOTSOCK] The large message was truncated";
        break;
    case WSAEOPNOTSUPP:
        str = "[WSAEOPNOTSUPP] OOB data is not supported";
        break;
    case WSAEADDRINUSE:
        str = "[WSAEADDRINUSE] Address is already in use";
        break;
    case WSAEADDRNOTAVAIL:
        str = "[WSAEADDRNOTAVAIL] Address is not valid in its context";
        break;
    case WSAENETDOWN:
        str = "[WSAENETDOWN] The network subsystem has failed";
        break;
    case WSAENETRESET:
        str = "[WSAENETRESET] The connection has been broken";
        break;
    case WSAECONNABORTED:
        str = "[WSAECONNABORTED] The connection was aborted";
        break;
    case WSAECONNRESET:
        str = "[WSAECONNRESET] Connection dropped";
        break;
    case WSAEISCONN:
        str = "[WSAEISCONN] The socket is connected already.";
        break;
    case WSAENOTCONN:
        str = "[WSAENOTCONN] The socket is not connected";
        break;
    case WSAESHUTDOWN:
        str = "[WSAESHUTDOWN] The socket has been shut down";
        break;
    case WSAETIMEDOUT:
        str = "[WSAETIMEDOUT] The connection failed to respond";
        break;
    case WSAECONNREFUSED:
        str = "[WSAECONNREFUSED] The target machine actively refused it";
        break;
    case WSANOTINITIALISED:
        str = "[WSANOTINITIALISED] WSAStartup is not called";
        break;
    case NET_SOCKETINVALID:
        str = "[NET_SOCKETINVALID] invalid socket";
        break;
    case NET_SOCKETNOTBOUND:
        str = "[NET_SOCKETNOTBOUND] bind failed";
        break;
    case NET_SOCKETNOTCONNECTED:
        str = "[NET_SOCKETNOTCONNECTED] connect failed";
        break;
    case NET_SOCKETNOTSHUTDOWN:
        str = "[NET_SOCKETNOTSHUTDOWN] shutdown failed";
        break;
    case NET_SOCKETNOTCLOSED:
        str = "[NET_SOCKETNOTCLOSED] close failed";
        break;
    case NET_ENTITYEXIST:
        str = "[NET_ENTITYEXIST] entity exists already";
        break;
    case NET_ENTITYNOTFOUND:
        str = "[NET_ENTITYNOTFOUND] Entity not found";
        break;
    case NET_CONTEXTNOTFOUND:
        str = "[NET_CONTEXTNOTFOUND] Context not found";
        break;
    case NET_IOCPNOTCREATED:
        str = "[NET_IOCPNOTCREATED] create IOCP failed";
        break;
    case NET_IOCPNOTASSOCIATED:
        str = "[NET_IOCPNOTASSOCIATED] associate IOCP failed";
        break;
    case NET_IOCPSENDPENDING:
        str = "[NET_IOCPSENDPENDING] maximum pending send";
        break;
    case NET_IOCPNOTPOST:
        str = "[NET_IOCPNOTPOST] post failed";
        break;
    case NET_IOCPNOTSENT:
        str = "[NET_IOCPNOTSENT] send failed";
        break;
    case NET_IOCPNOTRECV:
        str = "[NET_IOCPNOTRECV] receive failed";
        break;
    default:
        str = "[WSAERROR].";
        break;
    }
    return str;
}
