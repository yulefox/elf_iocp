/**
 * 由于 socket 可作为连接的唯一标识, 在上层使用时为淡化 socket 的印记, 将其称
 * 为 id, 因此各模块中 socket, id, type 等变量名称各异, 需加以辨识.
 */
#ifndef LIBDB_MYNET_H
#define LIBDB_MYNET_H

#include "public/net.hpp"

int mynet_init(void);
int mynet_fini(void);

void mynet_server_ready(int type, int id, bool flag);
bool mynet_server_isready(void);

/**
 * 处理网络事件
 */
int mynet_proc_event(SOCKET s, int type, int res, void *args);

/**
 * 处理网络消息.
 */
int mynet_proc_msg(void);

int mynet_send(CMessage &msg, net_type t);

#endif /* !LIBDB_MYNET_H */
