#include "atomic.h"
#include "pool/pool.h"
#include <windows.h>

int
atomic_inc(volatile int *n)
{
    return InterlockedIncrement((volatile LONG *)n);
}

int
atomic_dec(volatile int *n)
{
    return InterlockedDecrement((volatile LONG *)n);
}

int
atomic_add(volatile int *n, int val)
{
    return InterlockedExchangeAdd((volatile LONG *)n, (LONG)val);
}

int
atomic_sub(volatile int *n, int val)
{
    return InterlockedExchangeAdd((volatile LONG *)n, (LONG)(-val));
}

int
atomic_xchg(volatile int *dst, volatile int src)
{
    return InterlockedExchange((volatile LONG *)dst, (LONG)src);
}

void *
atomic_ptr_xchg(void **dst, void *src)
{
#pragma warning(push)
#pragma warning(disable : 4311 4312)
    return InterlockedExchangePointer(dst, src);
#pragma warning(pop)
}

void *
atomic_ptr_cas(void **dst, void *src, void *cmp)
{
    return InterlockedCompareExchangePointer(dst, src, cmp);
}
