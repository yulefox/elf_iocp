#include "mynet.h"
// 
#include "public/thread/mutex.h"
#include "../game/libdb/MsgType.h"

static mutex_t lock;
static bool server_ready;
static std::map<int, int> peers;
static std::map<int, int> servers;
static std::deque<CMessage *> msg_queue;

static int mynet_peer_insert(int id, int sock);
static int mynet_peer_remove(int id, int sock);
static int mynet_message_push(CMessage *msg);
static int mynet_message_proc(CMessage *msg);

extern void message_onlogin(CMessage *msg);
extern void message_onserver(CMessage *msg);
extern void OnDBMessage(CMessage* msg);          // 相应数据库操作消息
extern int DBSocket;
int
mynet_init(void)
{
    net_register(mynet_proc_event);
    mutex_init(&lock);
    net_init();
    server_ready = false;
    net_entity_start(256);
    return 0;
}

int
mynet_fini(void)
{
    net_fini();
    mutex_fini(&lock);
    return 0;
}

void
mynet_server_ready(int type, int id, bool flag)
{
    if (flag) {
        LOG_INFO("game.net", "Server is READY.");
        servers.insert(std::make_pair(type, id));
    } else {
        LOG_INFO("game.net", "Server is NOT ready.");
        servers.erase(type);
    }
    server_ready = flag;
}

bool
mynet_server_isready(void)
{
    return server_ready;
}

int
mynet_proc_event(SOCKET s, int type, int res, void *args)
{
    switch (type) {
    case NIT_CNCT:
        DBSocket = (int)s;
        net_onnotify(s, type, res, args);
        break;
    case NIT_ACPT:
        mynet_peer_insert(*(int *)args, (int)s);
        net_onnotify(s, type, res, args);
        break;
    case NIT_SNDF: /* 统计消息发送量 */
        break;
    case NIT_RCVF:
		{
			 CMessage *msg;
			 msg = new CMessage((message_t *)args, s);
			 mynet_message_push(msg);
			 OnDBMessage(msg);
		}
        break;
    case NIT_PCLS:
        mynet_peer_remove(*(int *)args, (int)s);
        net_onnotify(s, type, res, args);
        break;
    default:
        net_onnotify(s, type, res, args);
    }
    return 0;
}

int
mynet_proc_msg(void)
{
    std::deque<CMessage *> msgs;
    std::deque<CMessage *>::iterator itr;

    mutex_lock(lock);
    msgs = msg_queue;
    msg_queue.clear();
    mutex_unlock(lock);

    if (!msgs.empty()) {
        for (itr = msgs.begin(); itr != msgs.end(); ++itr) {
            mynet_message_proc(*itr);
        }
    }
    return 0;
}

int
mynet_send(CMessage &msg, net_type t)
{
    std::map<int, int>::iterator itr;

    itr = servers.find(t);
    if (itr != servers.end()) {
        //msg.Send(itr->second);
    }
    return 0;
}

int
mynet_peer_insert(int type, int sock)
{
    SOCKADDR_IN addr;
    int addr_len = sizeof(addr);

    getpeername(sock, (struct sockaddr *)&addr, &addr_len);
    switch (type) {
    case NET_SERVER_WORLD_SERVER:
        LOG_TRACE("game.net", "WORLD SERVER(%s:%d %d) is accepted.",
                  inet_ntoa(addr.sin_addr),
                  ntohs(addr.sin_port),
                  sock);
        break;
    case NET_SERVER_GAME_CLIENT:
        LOG_TRACE("game.net", "GAME CLIENT(%s:%d %d) is accepted.",
                  inet_ntoa(addr.sin_addr),
                  ntohs(addr.sin_port),
                  sock);
        break;
    default:
        LOG_WARN("game.net", "UNKNOWN PEER %d(%s:%d %d) is accepted.",
                 type,
                 inet_ntoa(addr.sin_addr),
                 ntohs(addr.sin_port),
                 sock);
    }
    peers.insert(std::make_pair(sock, type));
    return 0;
}

int
mynet_peer_remove(int type, int sock)
{
    return 0;
}

int
mynet_message_push(CMessage *msg)
{
    mutex_lock(lock);
    msg_queue.push_back(msg);
    mutex_unlock(lock);
    return 0;
}

int
mynet_message_proc(CMessage *msg)
{
	int cate;

	cate = msg->GetType() & 0xFFFFFF00;
	switch (cate) 
	{
	case MSG_DB_LOAD_RET:
	case MSG_DB_SAVE_RET:
		{
			OnDBMessage(msg);
		}
	break;
	default:
	LOG_WARN("game.net", "UNKNOWN MESSAGE CATEGORY: 0x%08X.",cate);
	}
	delete msg;

	return 0;
}
