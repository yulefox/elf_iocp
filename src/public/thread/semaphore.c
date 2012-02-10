/*
 * Copyright 2011, Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include "semaphore.h"
#include <assert.h>

int sem_init(sem_t *s)
{
    int rc = 0;

#ifdef _WIN32
    *s = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(*s != NULL);
    if (*s == NULL) rc = -1;
#else
    rc = pthread_mutex_init(s, NULL);
    assert(rc);
    rc = pthread_mutex_lock(s);
    assert(rc);
#endif /* _WIN32 */

    return rc;
}

void
sem_fini(sem_t s)
{
#ifdef _WIN32
    BOOL rc = CloseHandle(s);
    assert(rc);
#else
    int rc = pthread_mutex_unlock(&s);
    assert(rc);
    rc = pthread_mutex_destroy(&s);
    assert(rc);
#endif /* _WIN32 */
}

int
sem_wait(sem_t s)
{
    int rc;

#ifdef _WIN32
    rc = WaitForSingleObject(s, 0);
    assert(rc != WAIT_FAILED);
#else
    rc = pthread_mutex_lock(&mutex);
    assert(rc);
#endif /* _WIN32 */
    return rc;
}

void
sem_post(sem_t s)
{
#ifdef _WIN32
    BOOL rc = SetEvent(s);
    assert(rc);
#else
    int rc = pthread_mutex_unlock(&mutex);
    assert(rc);
#endif /* _WIN32 */
}
