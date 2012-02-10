/**
 * @file net.h
 * @date 2011-09-01
 * @author Fox (yulefox at gmail.com)
 * 网络模块.
 * 网络底层套接字管理原则:
 * - 网络底层使用 entity_t 作为网络连接的实例;
 * - 逻辑层使用 peer_t 作为消息收发的实例;
 * - 服务器监听套接字由 entity_t 负责关闭;
 * - 客户端连接及逻辑层使用同一套接字, 连接成功后绑定到 IOCP, 该套接字由 iocp_t
 *   负责关闭;
 * a. 客户端逻辑层通信实例管理原则:
 * - 由逻辑层根据配置 ID 自行维护服务器实例对象, 多客户端时, 
 * 无论服务器还是客户端, 所有 peer_t 均由网络底层在建立连接时创建, 返回给逻辑层.
 */

#ifndef PUBLIC_NET_H
#define PUBLIC_NET_H

#pragma once

#include "public.h"
#include <winsock2.h>
#include "net/entity.h"
#include "net/message.h"
#include "net/raw.h"

#ifdef _DEBUG
#pragma comment(lib, "libnet_d.lib")
#else
#pragma comment(lib, "libnet.lib")
#endif /* _DEBUG */

BEGIN_C_DECLS

/**
 * Initialize with configure.
 * - log
 * - memory
 */
int net_init(void);
int net_fini(void);
int net_load(void);
int net_start(void);
void net_register(net_func func);
int net_callback(SOCKET s, int type, int res, void *args);
int net_onnotify(SOCKET s, int type, int res, void *args);

END_C_DECLS

#endif /* !PUBLIC_NET_H */