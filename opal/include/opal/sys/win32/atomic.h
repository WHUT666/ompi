/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2026      Amazon.com, Inc. or its affiliates.
 *                         All Rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 * SPDX-License-Identifier: BSD-3-Clause-Open-MPI
 *
 * Atomic operations for the native Windows (MSVC) build.  Implemented
 * with the _Interlocked* compiler intrinsics documented for x86/x64
 * and ARM64; all of them emit LOCK-prefixed (or LDXR/STXR) sequences
 * that imply full hardware barriers, so the relaxed/acquire/release
 * spellings all share the underlying operation.
 */

#ifndef OPAL_SYS_ATOMIC_WIN32_H
#define OPAL_SYS_ATOMIC_WIN32_H

#include <intrin.h>
#include <stdint.h>
#include <stdbool.h>

BEGIN_C_DECLS

/**********************************************************************
 *
 * Memory barriers
 *
 *********************************************************************/

/* A locked operation serves as the full fence; _mm_mfence() alone
 * would not prevent the compiler from sinking loads. */
static inline void opal_atomic_mb(void)
{
    volatile long barrier_var = 0;
    _InterlockedOr(&barrier_var, 0);
}

static inline void opal_atomic_wmb(void)
{
    /* x86/x64 stores are not reordered with other stores; a compiler
     * barrier is all we need */
    _ReadWriteBarrier();
}

static inline void opal_atomic_rmb(void)
{
    /* x86/x64 loads are not reordered with other loads */
    _ReadWriteBarrier();
}

/**********************************************************************
 *
 * Compare and swap
 *
 *********************************************************************/

static inline bool opal_atomic_compare_exchange_strong_32(opal_atomic_int32_t *addr,
                                                          int32_t *oldval, int32_t newval)
{
    int32_t prev =
        (int32_t) _InterlockedCompareExchange((volatile long *) addr, newval, *oldval);
    bool ret = (prev == *oldval);
    *oldval = prev;
    return ret;
}

static inline bool opal_atomic_compare_exchange_strong_64(opal_atomic_int64_t *addr,
                                                          int64_t *oldval, int64_t newval)
{
    int64_t prev = (int64_t) _InterlockedCompareExchange64((volatile long long *) addr,
                                                         newval, *oldval);
    bool ret = (prev == *oldval);
    *oldval = prev;
    return ret;
}

/* acquire/release variants map onto the same LOCKed op on
 * x86/x64/ARM64-with-atomics; the intrinsic already implies the
 * required ordering */
#define opal_atomic_compare_exchange_strong_acq_32 \
    opal_atomic_compare_exchange_strong_32
#define opal_atomic_compare_exchange_strong_rel_32 \
    opal_atomic_compare_exchange_strong_32
#define opal_atomic_compare_exchange_strong_acq_64 \
    opal_atomic_compare_exchange_strong_64
#define opal_atomic_compare_exchange_strong_rel_64 \
    opal_atomic_compare_exchange_strong_64

#define OPAL_HAVE_ATOMIC_COMPARE_EXCHANGE_32 1
#define OPAL_HAVE_ATOMIC_COMPARE_EXCHANGE_64 1

/**********************************************************************
 *
 * Swap
 *
 *********************************************************************/

static inline int32_t opal_atomic_swap_32(opal_atomic_int32_t *addr, int32_t newval)
{
    return (int32_t) _InterlockedExchange((volatile long *) addr, newval);
}

static inline int64_t opal_atomic_swap_64(opal_atomic_int64_t *addr, int64_t newval)
{
    return (int64_t) _InterlockedExchange64((volatile long long *) addr, newval);
}

#define OPAL_HAVE_ATOMIC_SWAP_32 1
#define OPAL_HAVE_ATOMIC_SWAP_64 1

/**********************************************************************
 *
 * Atomic spinlocks
 *
 *********************************************************************/

static inline void opal_atomic_lock_init(opal_atomic_lock_t *lock, int32_t value)
{
    *lock = value;
    opal_atomic_wmb();
}

static inline int opal_atomic_trylock(opal_atomic_lock_t *lock)
{
    int32_t unlocked = OPAL_ATOMIC_LOCK_UNLOCKED;
    return !opal_atomic_compare_exchange_strong_acq_32(lock, &unlocked,
                                                     OPAL_ATOMIC_LOCK_LOCKED);
}

static inline void opal_atomic_lock(opal_atomic_lock_t *lock)
{
    while (0 != opal_atomic_trylock(lock)) {
        YieldProcessor();
    }
}

static inline void opal_atomic_unlock(opal_atomic_lock_t *lock)
{
    opal_atomic_wmb();
    *lock = OPAL_ATOMIC_LOCK_UNLOCKED;
}

/**********************************************************************
 *
 * Fetch-op math
 *
 *********************************************************************/

/* add */
static inline int32_t opal_atomic_fetch_add_32(opal_atomic_int32_t *addr, int32_t delta)
{
    return (int32_t) _InterlockedExchangeAdd((volatile long *) addr, delta);
}

static inline int32_t opal_atomic_add_fetch_32(opal_atomic_int32_t *addr, int32_t delta)
{
    return opal_atomic_fetch_add_32(addr, delta) + delta;
}

static inline int64_t opal_atomic_fetch_add_64(opal_atomic_int64_t *addr, int64_t delta)
{
    return (int64_t) _InterlockedExchangeAdd64((volatile long long *) addr, delta);
}

static inline int64_t opal_atomic_add_fetch_64(opal_atomic_int64_t *addr, int64_t delta)
{
    return opal_atomic_fetch_add_64(addr, delta) + delta;
}

/* sub */
static inline int32_t opal_atomic_fetch_sub_32(opal_atomic_int32_t *addr, int32_t delta)
{
    return opal_atomic_fetch_add_32(addr, -delta);
}

static inline int32_t opal_atomic_sub_fetch_32(opal_atomic_int32_t *addr, int32_t delta)
{
    return opal_atomic_fetch_sub_32(addr, delta) - delta;
}

static inline int64_t opal_atomic_fetch_sub_64(opal_atomic_int64_t *addr, int64_t delta)
{
    return opal_atomic_fetch_add_64(addr, -delta);
}

static inline int64_t opal_atomic_sub_fetch_64(opal_atomic_int64_t *addr, int64_t delta)
{
    return opal_atomic_fetch_sub_64(addr, delta) - delta;
}

/* and / or / xor */
static inline int32_t opal_atomic_fetch_and_32(opal_atomic_int32_t *addr, int32_t value)
{
    return (int32_t) _InterlockedAnd((volatile long *) addr, value);
}

static inline int32_t opal_atomic_and_fetch_32(opal_atomic_int32_t *addr, int32_t value)
{
    return opal_atomic_fetch_and_32(addr, value) & value;
}

static inline int64_t opal_atomic_fetch_and_64(opal_atomic_int64_t *addr, int64_t value)
{
    return (int64_t) _InterlockedAnd64((volatile long long *) addr, value);
}

static inline int64_t opal_atomic_and_fetch_64(opal_atomic_int64_t *addr, int64_t value)
{
    return opal_atomic_fetch_and_64(addr, value) & value;
}

static inline int32_t opal_atomic_fetch_or_32(opal_atomic_int32_t *addr, int32_t value)
{
    return (int32_t) _InterlockedOr((volatile long *) addr, value);
}

static inline int32_t opal_atomic_or_fetch_32(opal_atomic_int32_t *addr, int32_t value)
{
    return opal_atomic_fetch_or_32(addr, value) | value;
}

static inline int64_t opal_atomic_fetch_or_64(opal_atomic_int64_t *addr, int64_t value)
{
    return (int64_t) _InterlockedOr64((volatile long long *) addr, value);
}

static inline int64_t opal_atomic_or_fetch_64(opal_atomic_int64_t *addr, int64_t value)
{
    return opal_atomic_fetch_or_64(addr, value) | value;
}

static inline int32_t opal_atomic_fetch_xor_32(opal_atomic_int32_t *addr, int32_t value)
{
    return (int32_t) _InterlockedXor((volatile long *) addr, value);
}

static inline int32_t opal_atomic_xor_fetch_32(opal_atomic_int32_t *addr, int32_t value)
{
    return opal_atomic_fetch_xor_32(addr, value) ^ value;
}

static inline int64_t opal_atomic_fetch_xor_64(opal_atomic_int64_t *addr, int64_t value)
{
    return (int64_t) _InterlockedXor64((volatile long long *) addr, value);
}

static inline int64_t opal_atomic_xor_fetch_64(opal_atomic_int64_t *addr, int64_t value)
{
    return opal_atomic_fetch_xor_64(addr, value) ^ value;
}

#define OPAL_HAVE_ATOMIC_MATH_32 1
#define OPAL_HAVE_ATOMIC_MATH_64 1

/* 128-bit compare exchange via cmpxchg16b (x86_64).  Only enabled when
 * the build provides an opal_int128_t type -- MSVC has no __int128, so
 * HAVE_OPAL_INT128_T is 0 here and this block compiles out. */
#if defined(HAVE_OPAL_INT128_T) && HAVE_OPAL_INT128_T \
    && (defined(_M_X64) || defined(_M_AMD64))
#    define OPAL_HAVE_ATOMIC_COMPARE_EXCHANGE_128 1

static inline bool opal_atomic_compare_exchange_strong_128(opal_atomic_int128_t *addr,
                                                           opal_int128_t *oldval,
                                                           opal_int128_t newval)
{
    /* _InterlockedCompareExchange128 performs cmpxchg16b; on failure
     * the observed value is written back through the comparand
     * pointer */
    char ok = _InterlockedCompareExchange128((volatile long long *) addr,
                                             (long long) (newval >> 64),
                                             (long long) (newval & 0xffffffffffffffffULL),
                                             (long long *) oldval);
    return ok != 0;
}
#endif

#include "opal/sys/atomic_impl_ptr_cswap.h"
#include "opal/sys/atomic_impl_ptr_swap.h"
#include "opal/sys/atomic_impl_minmax_math.h"
#include "opal/sys/atomic_impl_size_t_math.h"

END_C_DECLS

#endif /* OPAL_SYS_ATOMIC_WIN32_H */
