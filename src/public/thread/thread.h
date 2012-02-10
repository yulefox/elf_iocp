/**
 * Copyright (C) 2011 by Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 *
 * @file thread.h
 * @author Fox (yulefox at gmail.com)
 * @date 2011-07-08
 *
 * Thread.
 */

#ifndef ELF_THREAD_THREAD_H
#define ELF_THREAD_THREAD_H

#include "public/def/type.h"
#include "mutex.h"
#include "semaphore.h"

#ifndef _WIN32
#include <pthread.h>
#endif /* !_WIN32 */

BEGIN_C_DECLS

/**
 * Thread starts.
 * Return a thread id.
 */
ELF_API int thread_create(struct thread_t **t, thread_func func, void *args);

/**
 * Thread exits outside.
 */
ELF_API int thread_destroy(struct thread_t *t);

/**
 * Thread exits inside.
 */
ELF_API int thread_exit(struct thread_t *t);

ELF_API bool thread_isexit(void *t);

ELF_API void *thread_args(void *t);

END_C_DECLS

#endif /* !ELF_THREAD_THREAD_H */
