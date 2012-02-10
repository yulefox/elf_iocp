/**
 * Copyright (C) 2011 by Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 *
 * @file semaphore.h
 * @author Fox (yulefox at gmail.com)
 * @date 2011-07-09
 *
 * Semaphore.
 */

#ifndef ELF_THREAD_SEMAPHORE_H
#define ELF_THREAD_SEMAPHORE_H

#include "public/def.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif /* _WIN32 */

BEGIN_C_DECLS

#ifdef _WIN32
typedef HANDLE sem_t;
#else
typedef pthread_mutex_t sem_t;
#endif /* _WIN32 */

/**
 * Initialise the semaphore.
 */
ELF_API int sem_init(sem_t *s);

/**
 * Destroy the semaphore.
 */
ELF_API void sem_fini(sem_t s);

/**
 * Wait for the semaphore.
 * @return 0 if signaled.
 */
ELF_API int sem_wait(sem_t s);

/**
 * Post the semaphore.
 */
ELF_API void sem_post(sem_t s);

END_C_DECLS

#endif /* !ELF_THREAD_SEMAPHORE_H */
