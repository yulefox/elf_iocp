/**
 * @file net.hpp
 * @date 2011-11-05
 * @author Fox (yulefox at gmail.com)
 * 网络库的 C++ 适配层.
 */

#ifndef PUBLIC_NET_HPP
#define PUBLIC_NET_HPP

#pragma once

#include "net.h"
#include "game/user/user.h"

enum net_type {
    NET_SERVER_GAME_SERVER = 250,
    NET_SERVER_LOGIN_SERVER = 252,
    NET_SERVER_WORLD_SERVER = 254,
    NET_SERVER_GAME_CLIENT = 256,
	NET_CLIENT_AUTH_SERVER = 257,
	NET_CLIENT_ACC_SERVER = 259,
};

class CMessage {
public:
    CMessage(int type); /* 发送 */
    CMessage(message_t *msg, SOCKET s); /* 接收 */
    virtual ~CMessage(void);
	size_t GetTotalSize(void);
    int GetType(void);
    int SetType(int type); /* 仅为转发用 */
    int GetSocket(void);
	void Add(char val);
	void Add(int val);
	void Add(long val);
	void Add(float val);
	void Add(const char *val);
	void Add(const uid_t &val);
	void Add(const CGUID &val);
	void AddEx(void *val, size_t size);
    char GetChar(void);
    int GetInt(void);
    long GetLong(void);
    __int64 GetLONG64(void);
	float GetFloat(void);
	const char *GetStr(char *val, size_t size);
	bool GetUID(uid_t &uid);
	bool GetGUID(CGUID &val);
	void *GetEx(void *val, size_t size);
    void Send(int sock);
    void Send(net_type type);
    //void Send(const uid_t &uid);
	//bool GetDBWriteSet(tagDataBlockWriteSet &DBWriteSet);
	//bool GetDBReadSet(tagDataBlockReadSet &DBReadSet);
	//void Encrypt(const uchar kn[16][6]);	/// 对消息进行加密
	//void Decrypt(const uchar kn[16][6]);	/// 对消息进行解密

private:
    CMessage(void);
    message_t *m_msg;
    int m_sock;
};

#endif /* !PUBLIC_NET_HPP */
