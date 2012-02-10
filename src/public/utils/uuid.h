/**
 * Copyright (C) 2010 - 2011 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 *
 * @file uuid.h
 * @author Fox (yulefox at gmail.com)
 * @date 2012-01-05
 */

#ifndef PUBLIC_UTILS_UUID_H
#define PUBLIC_UTILS_UUID_H

#include "public/def.h"
#include "public/def/wincrt.h"

BEGIN_C_DECLS

int uuid_init(void);
int uuid_fini(void);
int uuid_create(uuid_t *uuid);
int uuid_to_str(uuid_t *uuid, unsigned char *str);
int uuid_from_str(uuid_t *uuid, unsigned char *str);
int uuid_str_free(unsigned char **str);

END_C_DECLS

#endif /* !PUBLIC_UTILS_UUID_H */
