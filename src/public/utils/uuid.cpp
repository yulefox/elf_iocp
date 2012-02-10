/*
 * Copyright (C) 2011 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include "uuid.h"

int
uuid_init(void)
{
	CoInitialize(NULL);
    return 0;
}
int
uuid_fini(void)
{
	CoUninitialize();
    return 0;
}

int
uuid_create(uuid_t *uuid)
{
    int res = -1;

    if (S_OK == CoCreateGuid(uuid)) {
		res = 0;
    }
	return res;
}

int uuid_to_str(uuid_t *uuid, unsigned char **str)
{
    int res = -1;

    if (RPC_S_OK == UuidToString(uuid, str)) {
		res = 0;
    }
	return res;
}

int uuid_from_str(uuid_t *uuid, unsigned char *str)
{
    int res = -1;

    if (RPC_S_OK == UuidFromString(str, uuid)) {
		res = 0;
    }
	return res;
}

int
uuid_str_free(unsigned char **str)
{
    int res = -1;

    if (RPC_S_OK == RpcStringFree(str)) {
		res = 0;
    }
	return res;
}
