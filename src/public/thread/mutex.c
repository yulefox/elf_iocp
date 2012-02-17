/*
 * Copyright 2011, Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include "mutex.h"

int
mutex_init(mutex_t *m)
{
    int rc = 0;

#ifdef _WIN32
    InitializeCriticalSectionAndSpinCount(m, 1000);
#else
    rc = pthread_mutex_init(m, NULL);
    if (rc) assert(rc);
#endif /* _WIN32 */

    return rc;
}

void
mutex_fini(mutex_t *m)
{
#ifdef _WIN32
    DeleteCriticalSection(m);
#else
    int rc = pthread_mutex_destroy(m);
    if (rc) assert(rc);
#endif /* _WIN32 */
}

void
mutex_lock(mutex_t *m)
{
#ifdef _WIN32
    EnterCriticalSection(m);
#else
    int rc = pthread_mutex_lock(m);
    if (rc) assert(rc);
#endif /* _WIN32 */
}

void
mutex_unlock(mutex_t *m)
{
#ifdef _WIN32
    LeaveCriticalSection(m);
#else
    int rc = pthread_mutex_unlock(m);
    if (rc) assert(rc);
#endif /* _WIN32 */
}
