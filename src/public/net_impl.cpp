#include "net.hpp"
#include "game/user/user.h"

CMessage::CMessage(int type)
{
    m_msg = net_message_create(type);
    m_sock = INVALID_SOCKET;
}

CMessage::CMessage(message_t *msg, SOCKET s)
{
    m_msg = msg;
    m_sock = (int)s;
}

CMessage::~CMessage(void)
{
    // 发送消息时, 消息体的释放由网络库完成
    // 接收消息时, 消息体的释放由析构函数完成
    if (m_sock != INVALID_SOCKET) {
        net_message_destroy(m_msg);
    }
}

size_t CMessage::GetTotalSize(void)
{
    size_t size;

    net_message_get_size(m_msg, &size);
    return size;
}

int CMessage::GetType(void)
{
    int type;

    net_message_get_type(m_msg, &type);
    return type;
}

int CMessage::SetType(int type)
{
    net_message_set_type(m_msg, type);
    m_sock = INVALID_SOCKET;
    return 0;
}

int CMessage::GetSocket(void)
{
    return m_sock;
}

void CMessage::Add(char val)
{
    net_message_append_raw(m_msg, &val, sizeof(val));
}

void CMessage::Add(int val)
{
    net_message_append_raw(m_msg, &val, sizeof(val));
}

void CMessage::Add(long val)
{
    net_message_append_raw(m_msg, &val, sizeof(val));
}

void CMessage::Add(float val)
{
    net_message_append_raw(m_msg, &val, sizeof(val));
}

void CMessage::Add(const char *val)
{
    net_message_append_str(m_msg, val);
}

void CMessage::Add(const uid_t &val)
{
    net_message_append_raw(m_msg, (void *)&val, sizeof(val));
}

void CMessage::Add(const CGUID &val)
{
    net_message_append_raw(m_msg, (void *)&val, sizeof(val));
}

void CMessage::AddEx(void *val, size_t size)
{
    net_message_append_size(m_msg, size);
    net_message_append_raw(m_msg, val, size);
}

char CMessage::GetChar(void)
{
    char val;

    net_message_retrive_raw(m_msg, &val, sizeof(val));
	return val;
}

int CMessage::GetInt(void)
{
    return net_message_retrive_int(m_msg);
}

long CMessage::GetLong(void)
{
    long val;

    net_message_retrive_raw(m_msg, &val, sizeof(val));
	return val;
}

__int64 CMessage::GetLONG64(void)
{
    return net_message_retrive_int64(m_msg);
}

float CMessage::GetFloat(void)
{
    return net_message_retrive_float(m_msg);
}

bool CMessage::GetUID(uid_t &val)
{
    net_message_retrive_raw(m_msg, &val, sizeof(val));
    return true;
}

bool CMessage::GetGUID(CGUID &val)
{
    net_message_retrive_raw(m_msg, &val, sizeof(val));
    return true;
}

const char *CMessage::GetStr(char *val, size_t size)
{
    assert(strlen(net_message_retrive_str(m_msg, val)) < size);
    return val;
}

void *CMessage::GetEx(void *val, size_t size)
{
    size_t real_size;

    real_size = net_message_retrive_size(m_msg);
    real_size = min(real_size, size);
    net_message_retrive_raw(m_msg, val, real_size);
    return val;
}

void CMessage::Send(int sock)
{
    net_send(sock, m_msg);
}

void CMessage::Send(net_type type)
{
    int sock;

    sock = net_entity_socket(type);
    Send(sock);
}

// void CMessage::Send(const uid_t &uid)
// {
//     struct user_t *user;
// 
//     user_find(&user, uid);
//     assert(user);
//     Send(user->sock);
// }
