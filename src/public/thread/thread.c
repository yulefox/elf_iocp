/*
 * Copyright 2011, Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include "thread.h"
#include "public/atomic.h"
#include "public/pool/pool.h"
#include <assert.h>

#ifdef _WIN32
#   include <process.h>
#else
#   include <pthread.h>
#endif /* _WIN32 */

struct thread_t {
#ifdef _WIN32
    HANDLE tid;
#else
    pthread_t tid;
#endif /* _WIN32 */
    volatile bool exit;
    void *args;
};

int
thread_create(struct thread_t **t, thread_func func, void *args)
{
    (*t) = (struct thread_t *)pool_alloc(sizeof(**t));
    (*t)->exit = false;
    (*t)->args = args;
#ifdef _WIN32
    (*t)->tid = (HANDLE)_beginthreadex(NULL, 0, func, *t, 0, NULL);
    assert((*t)->tid != NULL);
#else
    int rc;

    rc = pthread_create(&((*t)->tid), NULL, func, *t);
    assert(!rc);
#endif /* _WIN32 */

    return 0;
}

int
thread_destroy(struct thread_t *t)
{
    if (t != NULL) {
        atomic_xchg((volatile int *)&(t->exit), true);
#ifdef _WIN32
        assert(WaitForSingleObject(t->tid, INFINITE) != WAIT_FAILED);
        assert(CloseHandle(t->tid));
#else
        assert(!pthread_join(tid, NULL));
#endif /* _WIN32 */
        pool_free(t);
    }
    return 0;
}

int
thread_exit(struct thread_t *t)
{
    if (t != NULL) {
        atomic_xchg((volatile int *)&(t->exit), true);
    }
    return 0;
}

bool thread_isexit(void *t)
{
    return ((struct thread_t *)t)->exit;
}

void *thread_args(void *t)
{
    return ((struct thread_t *)t)->args;
}
