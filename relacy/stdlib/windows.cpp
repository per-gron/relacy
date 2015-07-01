/*  Relacy Race Detector
 *  Copyright (c) 2008-2010, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE.TXT in this distribution.
 */

#include "windows.hpp"

namespace rl
{

int rl_SwitchToThread(debug_info_param info)
{
    yield(1, info);
    return 1;
}

void rl_Sleep(unsigned long milliseconds, debug_info_param info)
{
    yield(milliseconds ? milliseconds : 1, info);
}



unsigned long rl_WaitForSingleObjectEx(rl_HANDLE obj, unsigned long timeout, int alertable, debug_info_param info)
{
    (void)alertable; //!!! not yet supported ? support it!
    //!!! support WAIT_IO_COMPLETION
    RL_VERIFY(false == alertable && "Alertable wait is not supported in WaitForSingleObject() yet");

    bool try_wait = (timeout == 0);
    bool is_timed = (timeout != rl_INFINITE);
    sema_wakeup_reason reason = static_cast<win_waitable_object*>(obj)->wait(try_wait, is_timed, info);
    if (reason == sema_wakeup_reason_success) {
        return rl_WAIT_OBJECT_0;
    }
    else if (reason == sema_wakeup_reason_timeout) {
        return rl_WAIT_TIMEOUT;
    }
    else if (reason == sema_wakeup_reason_failed) {
        return rl_WAIT_TIMEOUT;
    }
    RL_VERIFY(false);
    return rl_WAIT_FAILED;
}

unsigned long rl_WaitForSingleObject(rl_HANDLE obj, unsigned long timeout, debug_info_param info)
{
    return rl_WaitForSingleObjectEx(obj, timeout, 0, info);
}

unsigned long rl_WaitForMultipleObjectsEx(unsigned long count, rl_HANDLE* objects, int wait_all, unsigned long timeout, int alertable, debug_info_param info)
{
    (void)alertable; //!!!
    //!!! support WAIT_IO_COMPLETION
    RL_VERIFY(false == alertable && "Alertable wait is not supported in WaitForMultipleObjects() yet");

    bool try_wait = (timeout == 0);
    bool is_timed = (timeout != rl_INFINITE);
    win_waitable_object** obj = reinterpret_cast<win_waitable_object**>(objects);
    size_t signaled = 0;
    sema_wakeup_reason reason = wait_for_multiple_objects(signaled, count, obj, !!wait_all, try_wait, is_timed, info);
    if (reason == sema_wakeup_reason_success)
        return rl_WAIT_OBJECT_0 + (int)signaled;
    else if (reason == sema_wakeup_reason_timeout)
        return rl_WAIT_TIMEOUT;
    RL_VERIFY(false);
    return rl_WAIT_FAILED;
}

unsigned long rl_WaitForMultipleObjects(unsigned long count, rl_HANDLE* objects, int wait_all, unsigned long timeout, debug_info_param info)
{
    return rl_WaitForMultipleObjectsEx(count, objects, wait_all, timeout, 0, info);
}

unsigned long rl_SignalObjectAndWait(rl_HANDLE obj_to_signal,
                                     rl_HANDLE obj_to_wait,
                                     unsigned long timeout,
                                     int alertable,
                                     debug_info_param info)
{
    bool result = static_cast<win_waitable_object*>(obj_to_signal)->signal(info);
    if (false == result)
        return result ? 1 : 0;
    preemption_disabler pd (ctx());
    return rl_WaitForSingleObjectEx(obj_to_wait, timeout, alertable, info);
}


rl_HANDLE rl_CreateSemaphore(void* /*security*/, long initial_count, long max_count, void const* /*name*/, debug_info_param info)
{
    void* mem = ctx().alloc(sizeof(semaphore<sem_tag_win>), false, info);
    semaphore<sem_tag_win>* sema = new (mem) semaphore<sem_tag_win>;
    sema->init(false, initial_count, max_count, info);
    return sema;
}

int rl_CloseHandle(rl_HANDLE h, debug_info_param info)
{
    h->deinit(info);
    h->~win_object();
    (ctx().free)(h, false, info); //!!! rename free because of the define
    return 1;
}

int rl_ReleaseSemaphore(rl_HANDLE sema, long count, long* prev_count, debug_info_param info)
{
    unsigned prev = 0;
    bool result = static_cast<semaphore<sem_tag_win>*>(sema)->post(count, prev, info);
    if (prev_count)
        prev_count[0] = prev;
    return result ? 1 : 0;
}




rl_HANDLE rl_CreateEvent(void* /*security*/, int manual_reset, int initial_state, void const* /*name*/, debug_info_param info)
{
    void* mem = ctx().alloc(sizeof(generic_event), false, info);
    generic_event* ev = new (mem) generic_event;
    ev->init(!!manual_reset, !!initial_state, info);
    return ev;
}

int rl_SetEvent(rl_HANDLE ev, debug_info_param info)
{
    static_cast<generic_event*>(ev)->set(info);
    return 1;
}

int rl_ResetEvent(rl_HANDLE ev, debug_info_param info)
{
    static_cast<generic_event*>(ev)->reset(info);
    return 1;
}

int rl_PulseEvent(rl_HANDLE ev, debug_info_param info)
{
    static_cast<generic_event*>(ev)->pulse(info);
    return 1;
}



void rl_InitializeCriticalSection(rl_CRITICAL_SECTION* m, debug_info_param info)
{
    m->init(false, true, false, false, info);
}

int rl_InitializeCriticalSectionAndSpinCount(rl_CRITICAL_SECTION* m, unsigned long spin_count, debug_info_param info)
{
    (void)spin_count;
    m->init(false, true, false, false, info);
    return 1;
}

int rl_InitializeCriticalSectionEx(rl_CRITICAL_SECTION* m, unsigned long spin_count, unsigned long flags, debug_info_param info)
{
    (void)spin_count;
    (void)flags;
    m->init(false, true, false, false, info);
    return 1;
}

void rl_DeleteCriticalSection(rl_CRITICAL_SECTION* m, debug_info_param info)
{
    m->deinit(info);
}

void rl_EnterCriticalSection(rl_CRITICAL_SECTION* m, debug_info_param info)
{
    m->lock_exclusive(info);
}

int rl_TryEnterCriticalSection(rl_CRITICAL_SECTION* m, debug_info_param info)
{
    return m->try_lock_exclusive(info) ? 1 : 0;
}

void rl_LeaveCriticalSection(rl_CRITICAL_SECTION* m, debug_info_param info)
{
    m->unlock_exclusive(info);
}

void rl_InitializeSRWLock(rl_SRWLOCK* lock, debug_info_param info)
{
    lock->init(true, false, false, false, info);
}

void rl_AcquireSRWLockExclusive(rl_SRWLOCK* lock, debug_info_param info)
{
    lock->lock_exclusive(info);
}

void rl_AcquireSRWLockShared(rl_SRWLOCK* lock, debug_info_param info)
{
    lock->lock_shared(info);
}

void rl_ReleaseSRWLockExclusive(rl_SRWLOCK* lock, debug_info_param info)
{
    lock->unlock_exclusive(info);
}

void rl_ReleaseSRWLockShared(rl_SRWLOCK* lock, debug_info_param info)
{
    lock->unlock_shared(info);
}

//!!!
void rl_DeleteSRWLock(rl_SRWLOCK* lock, debug_info_param info)
{
    lock->deinit(info);
}



rl_HANDLE rl_CreateMutex(void* /*security*/, int initial_owner, void const* /*name*/, debug_info_param info)
{
    void* mem = ctx().alloc(sizeof(rl_win_mutex), false, info);
    rl_win_mutex* mtx = new (mem) rl_win_mutex ();
    mtx->init(false, true, false, false, info);
    if (initial_owner)
        mtx->lock_exclusive(info);
    return mtx;
}

int rl_ReleaseMutex(rl_HANDLE mtx, debug_info_param info)
{
    static_cast<rl_win_mutex*>(mtx)->unlock_exclusive(info);
    return 1;
}



void rl_InitializeConditionVariable(rl_CONDITION_VARIABLE* cv, debug_info_param info)
{
    cv->init(false, info);
}

int rl_SleepConditionVariableCS(rl_CONDITION_VARIABLE* cv, rl_CRITICAL_SECTION* cs, unsigned long ms, debug_info_param info)
{
    cv->wait(*cs, ms != rl_INFINITE, info);
    return 0;
}

int rl_SleepConditionVariableSRW(rl_CONDITION_VARIABLE* cv, rl_SRWLOCK* lock, unsigned long ms, unsigned long flags, debug_info_param info)
{
    //!!! CONDITION_VARIABLE_LOCKMODE_SHARED
    (void)flags;
    cv->wait(*lock, ms != rl_INFINITE, info);
    return 0;
}

void rl_WakeAllConditionVariable(rl_CONDITION_VARIABLE* cv, debug_info_param info)
{
    cv->notify_all(info);
}

void rl_WakeConditionVariable(rl_CONDITION_VARIABLE* cv, debug_info_param info)
{
    cv->notify_one(info);
}

void rl_DeleteConditionVariable(rl_CONDITION_VARIABLE* cv, debug_info_param info)
{
    cv->deinit(info);
}


rl_HANDLE rl_CreateThread(void* security, unsigned stack_size, rl_WIN_START_ROUTINE fn, void* param, unsigned long creation_flags, unsigned long* thread_id, debug_info_param info)
{
    (void)security;
    (void)stack_size;
    (void)creation_flags;
    (void)thread_id;

    void* mem =
        ctx().alloc(sizeof(win32_thread_helper<rl_WIN_START_ROUTINE>), false, info);
    win32_thread_helper<rl_WIN_START_ROUTINE>* arg =
        new (mem) win32_thread_helper<rl_WIN_START_ROUTINE>;
    arg->fn = fn;
    arg->param = param;
    win_waitable_object* handle = ctx().create_thread(&win32_thread_helper<rl_WIN_START_ROUTINE>::thread, arg);
    return handle;
}


uintptr_t rl_beginthreadex(void *security, unsigned stack_size, rl_MSVCR_THREAD_ROUTINE start_address, void *arglist, unsigned initflag, unsigned* thrdaddr, debug_info_param info)
{
    (void)security;
    (void)stack_size;
    (void)initflag;
    (void)thrdaddr;

    void* mem = ctx().alloc(sizeof(win32_thread_helper<rl_MSVCR_THREAD_ROUTINE>), false, info);
    win32_thread_helper<rl_MSVCR_THREAD_ROUTINE>* arg =
        new (mem) win32_thread_helper<rl_MSVCR_THREAD_ROUTINE>;
    arg->fn = start_address;
    arg->param = arglist;
    win_waitable_object* handle = ctx().create_thread(&win32_thread_helper<rl_MSVCR_THREAD_ROUTINE>::thread, arg);
    return (uintptr_t)handle;
}

unsigned long rl_SetThreadAffinityMask(rl_HANDLE th, unsigned long affinity_mask, debug_info_param info)
{
    (void)(th);
    (void)(affinity_mask);
    (void)info;
    return 0;
}

int rl_SuspendThread(rl_HANDLE th, debug_info_param info)
{
    (void)th;
    (void)info;
    return 1;
}

int rl_ResumeThread(rl_HANDLE th, debug_info_param info)
{
    (void)th;
    (void)info;
    return 1;
}

unsigned long GetLastError()
{
    return (unsigned long)get_errno();
}

void SetLastError(unsigned long value)
{
    set_errno((int)value);
}

void rl_FlushProcessWriteBuffers(debug_info_param info)
{
    systemwide_fence(info);
}

}
