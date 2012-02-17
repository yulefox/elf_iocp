/**
 * Copyright (C) 2011 by Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 *
 * @file mutex.h
 * @author Fox (yulefox at gmail.com)
 * @date 2011-07-09
 *
 * Mutex.
 */

#ifndef ELF_THREAD_MUTEX_H
#define ELF_THREAD_MUTEX_H

#include "public/def.h"

BEGIN_C_DECLS

#ifdef _WIN32
#include "public/def/wincrt.h"
typedef CRITICAL_SECTION mutex_t;
#else
typedef pthread_mutex_t mutex_t;
#endif /* _WIN32 */

ELF_API int mutex_init(mutex_t *m);

ELF_API void mutex_fini(mutex_t *m);

ELF_API void mutex_lock(mutex_t *m);

ELF_API void mutex_unlock(mutex_t *m);

END_C_DECLS

#endif /* !ELF_THREAD_MUTEX_H */
