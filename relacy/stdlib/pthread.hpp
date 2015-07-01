/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#pragma once

#include "mutex.hpp"
#include "condition_variable.hpp"
#include "semaphore.hpp"


namespace rl
{

enum RL_POSIX_ERROR_CODE
{
    RL_SUCCESS,
    RL_EINVAL,
    RL_ETIMEDOUT,
    RL_EBUSY,
    RL_EINTR,
    RL_EAGAIN,
    RL_EWOULDBLOCK,
};


void rl_sched_yield(debug_info_param info);


typedef win_waitable_object* rl_pthread_t;
typedef void* rl_pthread_attr_t;

int rl_pthread_create(rl_pthread_t* th, rl_pthread_attr_t* attr, void* (*func) (void*), void* arg, debug_info_param info);

rl_pthread_t rl_pthread_self(debug_info_param info);

int rl_pthread_join(rl_pthread_t th, void** res, debug_info_param info);



struct sem_tag_pthread;
typedef semaphore<sem_tag_pthread> rl_sem_t;

int rl_sem_init(rl_sem_t* sema, int /*pshared*/, unsigned int initial_count, debug_info_param info);

int rl_sem_destroy(rl_sem_t* sema, debug_info_param info);

int rl_sem_wait(rl_sem_t* sema, debug_info_param info);

int rl_sem_trywait(rl_sem_t* sema, debug_info_param info);

int rl_sem_post(rl_sem_t* sema, debug_info_param info);

int rl_sem_getvalue(rl_sem_t* sema, int* value, debug_info_param info);



struct mutex_tag_pthread_mtx;
typedef generic_mutex<mutex_tag_pthread_mtx> rl_pthread_mutex_t;

struct rl_pthread_mutexattr_t
{
    bool is_recursive_;
};

enum RL_PTHREAD_MUTEX_TYPE
{
    RL_PTHREAD_MUTEX_NORMAL,
    RL_PTHREAD_MUTEX_ERRORCHECK,
    RL_PTHREAD_MUTEX_RECURSIVE,
    RL_PTHREAD_MUTEX_DEFAULT,
};

int rl_pthread_mutexattr_init(rl_pthread_mutexattr_t* attr, debug_info_param info);

int rl_pthread_mutexattr_destroy(rl_pthread_mutexattr_t* attr, debug_info_param info);

int rl_pthread_mutexattr_settype(rl_pthread_mutexattr_t* attr, int type, debug_info_param info);

int rl_pthread_mutex_init(rl_pthread_mutex_t* m, rl_pthread_mutexattr_t const* attr, debug_info_param info);

int rl_pthread_mutex_destroy(rl_pthread_mutex_t* m, debug_info_param info);

int rl_pthread_mutex_lock(rl_pthread_mutex_t* m, debug_info_param info);

int rl_pthread_mutex_timedlock(rl_pthread_mutex_t* m, const void* abs_timeout, debug_info_param info);

int rl_pthread_mutex_try_lock(rl_pthread_mutex_t* m, debug_info_param info);

int rl_pthread_mutex_unlock(rl_pthread_mutex_t* m, debug_info_param info);



struct mutex_tag_pthread_rwlock;
typedef generic_mutex<mutex_tag_pthread_rwlock> rl_pthread_rwlock_t;

int rl_pthread_rwlock_init(rl_pthread_rwlock_t* lock, void const* /*attr*/, debug_info_param info);

int rl_pthread_rwlock_destroy(rl_pthread_rwlock_t* lock, debug_info_param info);

int rl_pthread_rwlock_rdlock(rl_pthread_rwlock_t* lock, debug_info_param info);

int rl_pthread_rwlock_tryrdlock(rl_pthread_rwlock_t* lock, debug_info_param info);

int rl_pthread_rwlock_wrlock(rl_pthread_rwlock_t* lock, debug_info_param info);

int rl_pthread_rwlock_trywrlock(rl_pthread_rwlock_t* lock, debug_info_param info);

int rl_pthread_rwlock_unlock(rl_pthread_rwlock_t* lock, debug_info_param info);




struct condvar_tag_pthread;
typedef condvar<condvar_tag_pthread> rl_pthread_cond_t;
typedef int rl_pthread_condattr_t;

int rl_pthread_cond_init(rl_pthread_cond_t* cv, rl_pthread_condattr_t* /*condattr*/, debug_info_param info);

int rl_pthread_cond_destroy(rl_pthread_cond_t* cv, debug_info_param info);

int rl_pthread_cond_broadcast(rl_pthread_cond_t* cv, debug_info_param info);

int rl_pthread_cond_signal(rl_pthread_cond_t* cv, debug_info_param info);

int rl_pthread_cond_timedwait(rl_pthread_cond_t* cv, rl_pthread_mutex_t* m, void const* /*timespec*/, debug_info_param info);

int rl_pthread_cond_wait(rl_pthread_cond_t* cv, rl_pthread_mutex_t* m, debug_info_param info);




enum RL_FUTEX_OP
{
    RL_FUTEX_WAIT,
    RL_FUTEX_WAKE,
};

int rl_int_futex_impl(context& c,
                      atomic<int>* uaddr,
                      int op,
                      int val,
                      struct timespec const* timeout,
                      atomic<int>* uaddr2,
                      int val3,
                      debug_info_param info);

struct futex_event
{
    void* addr_;
    int   op_;
    int   val_;
    bool  timeout_;
    int   res_;

    void output(std::ostream& s) const;
};

int rl_futex(atomic<int>* uaddr,
             int op,
             int val,
             struct timespec const* timeout,
             atomic<int>* uaddr2,
             int val3,
             debug_info_param info);

}



#ifdef EINVAL
#   undef EINVAL
#endif
#define EINVAL                  rl::RL_EINVAL

#ifdef ETIMEDOUT
#   undef ETIMEDOUT
#endif
#define ETIMEDOUT               rl::RL_ETIMEDOUT

#ifdef EBUSY
#   undef EBUSY
#endif
#define EBUSY                   rl::RL_EBUSY

#ifdef EINTR
#   undef EINTR
#endif
#define EINTR                   rl::RL_EINTR

#ifdef EAGAIN
#   undef EAGAIN
#endif
#define EAGAIN                  rl::RL_EAGAIN

#ifdef EWOULDBLOCK
#   undef EWOULDBLOCK
#endif
#define EWOULDBLOCK                  rl::RL_EWOULDBLOCK

#define sched_yield() \
 rl::rl_sched_yield($)

#define pthread_yield() \
 rl::rl_sched_yield($)



#define pthread_t rl::rl_pthread_t
#define pthread_attr_t rl::rl_pthread_attr_t

#define pthread_create(th, attr, func, arg) \
 rl::rl_pthread_create(th, attr, func, arg, $)

#define pthread_self() \
 rl::rl_pthread_self($)

#define pthread_join(th, res) \
 rl::rl_pthread_join(th, res, $)




#define sem_t rl::rl_sem_t

#define sem_init(sema, pshared, initial_count)\
 rl::rl_sem_init(sema, pshared, initial_count, $)

#define sem_destroy(sema)\
 rl::rl_sem_destroy(sema, $)

#define sem_wait(sema)\
 rl::rl_sem_wait(sema, $)

#define sem_trywait(sema)\
 rl::rl_sem_trywait(sema, $)

#define sem_post(sema)\
rl::rl_sem_post(sema, $)

#define sem_getvalue(sema, pvalue)\
 rl::rl_sem_getvalue(sema, pvalue, $)





#define pthread_mutex_t             rl::rl_pthread_mutex_t
#define pthread_mutexattr_t         rl::rl_pthread_mutexattr_t

#ifdef PTHREAD_MUTEX_NORMAL
#   undef PTHREAD_MUTEX_NORMAL
#   undef PTHREAD_MUTEX_ERRORCHECK
#   undef PTHREAD_MUTEX_RECURSIVE
#   undef PTHREAD_MUTEX_DEFAULT
#endif

#define PTHREAD_MUTEX_NORMAL        rl::RL_PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_ERRORCHECK    rl::RL_PTHREAD_MUTEX_ERRORCHECK
#define PTHREAD_MUTEX_RECURSIVE     rl::RL_PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_DEFAULT       rl::RL_PTHREAD_MUTEX_DEFAULT

#define pthread_mutexattr_init(attr) \
 rl::rl_pthread_mutexattr_init(attr, $)

#define pthread_mutexattr_destroy(attr) \
 rl::rl_pthread_mutexattr_destroy(attr, $)

#define pthread_mutexattr_settype(attr, type) \
 rl::rl_pthread_mutexattr_settype(attr, type, $)

#define pthread_mutex_init(m, attr) \
 rl::rl_pthread_mutex_init(m, attr, $)

#define pthread_mutex_destroy(m) \
 rl::rl_pthread_mutex_destroy(m, $)

#define pthread_mutex_lock(m) \
 rl::rl_pthread_mutex_lock(m, $)

#define pthread_mutex_timedlock(m, abs_timeout) \
 rl::rl_pthread_mutex_timedlock(m, abs_timeout, $)

#define pthread_mutex_try_lock(m) \
 rl::rl_pthread_mutex_try_lock(m, $)

#define pthread_mutex_unlock(m) \
 rl::rl_pthread_mutex_unlock(m, $)

#define pthread_rwlock_t rl::rl_pthread_rwlock_t

#define pthread_rwlock_init(lock, attr) \
 rl::rl_pthread_rwlock_init(lock, attr, $)

#define pthread_rwlock_destroy(lock) \
 rl::rl_pthread_rwlock_destroy(lock, $)

#define pthread_rwlock_rdlock(lock) \
 rl::rl_pthread_rwlock_rdlock(lock, $)

#define pthread_rwlock_tryrdlock(lock) \
 rl::rl_pthread_rwlock_tryrdlock(lock, $)

#define pthread_rwlock_wrlock(lock) \
 rl::rl_pthread_rwlock_wrlock(lock, $)

#define pthread_rwlock_trywrlock(lock) \
 rl::rl_pthread_rwlock_trywrlock(lock, $)

#define pthread_rwlock_unlock(lock) \
 rl::rl_pthread_rwlock_unlock(lock, $)




#define pthread_cond_t rl::rl_pthread_cond_t
#define pthread_condattr_t rl::rl_pthread_condattr_t

#define pthread_cond_init(cv, condattr) \
 rl::rl_pthread_cond_init(cv, condattr, $)

#define pthread_cond_destroy(cv) \
 rl::rl_pthread_cond_destroy(cv, $)

#define pthread_cond_broadcast(cv) \
 rl::rl_pthread_cond_broadcast(cv, $)

#define pthread_cond_signal(cv) \
 rl::rl_pthread_cond_signal(cv, $)

#define pthread_cond_timedwait(cv, m, timespec) \
 rl::rl_pthread_cond_timedwait(cv, m, timespec, $)

#define pthread_cond_wait(cv, m) \
 rl::rl_pthread_cond_wait(cv, m, $)



#ifdef FUTEX_WAKE
#   undef FUTEX_WAKE
#endif
#define FUTEX_WAKE                  rl::RL_FUTEX_WAKE

#ifdef FUTEX_WAIT
#   undef FUTEX_WAIT
#endif
#define FUTEX_WAIT                  rl::RL_FUTEX_WAIT

#define futex(uaddr, op, val, timeout, uaddr2, val3) \
 rl::rl_futex(uaddr, op, val, timeout, uaddr2, val3, $)
