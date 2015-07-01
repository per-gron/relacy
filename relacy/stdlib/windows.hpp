/*  Relacy Race Detector
 *  Copyright (c) 2008-2010, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE.TXT in this distribution.
 */

#pragma once

#include "mutex.hpp"
#include "condition_variable.hpp"
#include "semaphore.hpp"
#include "event.hpp"


namespace rl
{

static unsigned long const rl_INFINITE                   = (unsigned long)-1;
static unsigned long const rl_WAIT_FAILED                = (unsigned long)-1;
static unsigned long const rl_WAIT_OBJECT_0              = 100;
static unsigned long const rl_WAIT_TIMEOUT               = 1;
static unsigned long const rl_WAIT_IO_COMPLETION         = 2;
static unsigned long const rl_MAXIMUM_WAIT_OBJECTS       = wfmo_max_objects;

typedef win_object* rl_HANDLE;

int rl_SwitchToThread(debug_info_param info);

void rl_Sleep(unsigned long milliseconds, debug_info_param info);



unsigned long rl_WaitForSingleObjectEx(rl_HANDLE obj, unsigned long timeout, int alertable, debug_info_param info);

unsigned long rl_WaitForSingleObject(rl_HANDLE obj, unsigned long timeout, debug_info_param info);

unsigned long rl_WaitForMultipleObjectsEx(unsigned long count, rl_HANDLE* objects, int wait_all, unsigned long timeout, int alertable, debug_info_param info);

unsigned long rl_WaitForMultipleObjects(unsigned long count, rl_HANDLE* objects, int wait_all, unsigned long timeout, debug_info_param info);

unsigned long rl_SignalObjectAndWait(rl_HANDLE obj_to_signal,
                                     rl_HANDLE obj_to_wait,
                                     unsigned long timeout,
                                     int alertable,
                                     debug_info_param info);



struct sem_tag_win;

rl_HANDLE rl_CreateSemaphore(void* /*security*/, long initial_count, long max_count, void const* /*name*/, debug_info_param info);

int rl_CloseHandle(rl_HANDLE h, debug_info_param info);

int rl_ReleaseSemaphore(rl_HANDLE sema, long count, long* prev_count, debug_info_param info);



rl_HANDLE rl_CreateEvent(void* /*security*/, int manual_reset, int initial_state, void const* /*name*/, debug_info_param info);

int rl_SetEvent(rl_HANDLE ev, debug_info_param info);

int rl_ResetEvent(rl_HANDLE ev, debug_info_param info);

int rl_PulseEvent(rl_HANDLE ev, debug_info_param info);



struct mutex_tag_win_cs;
typedef generic_mutex<mutex_tag_win_cs> rl_CRITICAL_SECTION;

void rl_InitializeCriticalSection(rl_CRITICAL_SECTION* m, debug_info_param info);

int rl_InitializeCriticalSectionAndSpinCount(rl_CRITICAL_SECTION* m, unsigned long spin_count, debug_info_param info);

int rl_InitializeCriticalSectionEx(rl_CRITICAL_SECTION* m, unsigned long spin_count, unsigned long flags, debug_info_param info);

void rl_DeleteCriticalSection(rl_CRITICAL_SECTION* m, debug_info_param info);

void rl_EnterCriticalSection(rl_CRITICAL_SECTION* m, debug_info_param info);

int rl_TryEnterCriticalSection(rl_CRITICAL_SECTION* m, debug_info_param info);

void rl_LeaveCriticalSection(rl_CRITICAL_SECTION* m, debug_info_param info);

struct mutex_tag_win_srwl;
typedef generic_mutex<mutex_tag_win_srwl> rl_SRWLOCK;

void rl_InitializeSRWLock(rl_SRWLOCK* lock, debug_info_param info);

void rl_AcquireSRWLockExclusive(rl_SRWLOCK* lock, debug_info_param info);

void rl_AcquireSRWLockShared(rl_SRWLOCK* lock, debug_info_param info);

void rl_ReleaseSRWLockExclusive(rl_SRWLOCK* lock, debug_info_param info);

void rl_ReleaseSRWLockShared(rl_SRWLOCK* lock, debug_info_param info);

//!!!
void rl_DeleteSRWLock(rl_SRWLOCK* lock, debug_info_param info);


struct mutex_tag_win_mutex;
typedef generic_mutex<mutex_tag_win_mutex> rl_win_mutex;


rl_HANDLE rl_CreateMutex(void* /*security*/, int initial_owner, void const* /*name*/, debug_info_param info);

int rl_ReleaseMutex(rl_HANDLE mtx, debug_info_param info);



struct condvar_tag_win;
typedef condvar<condvar_tag_win> rl_CONDITION_VARIABLE;
static unsigned long const rl_CONDITION_VARIABLE_LOCKMODE_SHARED = 1;

void rl_InitializeConditionVariable(rl_CONDITION_VARIABLE* cv, debug_info_param info);

int rl_SleepConditionVariableCS(rl_CONDITION_VARIABLE* cv, rl_CRITICAL_SECTION* cs, unsigned long ms, debug_info_param info);

int rl_SleepConditionVariableSRW(rl_CONDITION_VARIABLE* cv, rl_SRWLOCK* lock, unsigned long ms, unsigned long flags, debug_info_param info);

void rl_WakeAllConditionVariable(rl_CONDITION_VARIABLE* cv, debug_info_param info);

void rl_WakeConditionVariable(rl_CONDITION_VARIABLE* cv, debug_info_param info);

void rl_DeleteConditionVariable(rl_CONDITION_VARIABLE* cv, debug_info_param info);



typedef unsigned long (RL_STDCALL *rl_WIN_START_ROUTINE)(void* param);
typedef unsigned (RL_STDCALL *rl_MSVCR_THREAD_ROUTINE)(void* param);

template<typename thread_fn_t>
struct win32_thread_helper
{
    thread_fn_t fn;
    void* param;

    static void* thread(void* p)
    {
        win32_thread_helper* self = (win32_thread_helper*)p;
        void* result = (void*)(uintptr_t)(self->fn(self->param));
        delete_impl(self, $);
        return result;
    }
};

rl_HANDLE rl_CreateThread(void* security, unsigned stack_size, rl_WIN_START_ROUTINE fn, void* param, unsigned long creation_flags, unsigned long* thread_id, debug_info_param info);

uintptr_t rl_beginthreadex(void *security, unsigned stack_size, rl_MSVCR_THREAD_ROUTINE start_address, void *arglist, unsigned initflag, unsigned* thrdaddr, debug_info_param info);

unsigned long rl_SetThreadAffinityMask(rl_HANDLE th, unsigned long affinity_mask, debug_info_param info);

int rl_SuspendThread(rl_HANDLE th, debug_info_param info);

int rl_ResumeThread(rl_HANDLE th, debug_info_param info);

unsigned long GetLastError();

void SetLastError(unsigned long value);

void rl_FlushProcessWriteBuffers(debug_info_param info);

}


#ifdef HANDLE
#   undef HANDLE
#endif
#define HANDLE rl::rl_HANDLE

#ifdef INFINITE
#   undef INFINITE
#endif
#define INFINITE rl::rl_INFINITE


#ifdef WAIT_FAILED
#   undef WAIT_FAILED
#endif
#define WAIT_FAILED rl::rl_WAIT_FAILED

#ifdef WAIT_OBJECT_0
#   undef WAIT_OBJECT_0
#endif
#define WAIT_OBJECT_0 rl::rl_WAIT_OBJECT_0

#ifdef WAIT_TIMEOUT
#   undef WAIT_TIMEOUT
#endif
#define WAIT_TIMEOUT rl::rl_WAIT_TIMEOUT

#ifdef WAIT_IO_COMPLETION
#   undef WAIT_IO_COMPLETION
#endif
#define WAIT_IO_COMPLETION rl::rl_WAIT_IO_COMPLETION

#ifdef MAXIMUM_WAIT_OBJECTS
#   undef MAXIMUM_WAIT_OBJECTS
#endif
#define MAXIMUM_WAIT_OBJECTS rl::rl_MAXIMUM_WAIT_OBJECTS



#define SwitchToThread() \
 rl::rl_SwitchToThread($)

#define Sleep(milliseconds) \
 rl::rl_Sleep(milliseconds, $)



#define CloseHandle(obj) \
 rl::rl_CloseHandle(obj, $)

#define WaitForSingleObject(obj, timeout) \
 rl::rl_WaitForSingleObject(obj, timeout, $)

#define WaitForMultipleObjects(count, objects, wait_all, timeout) \
 rl::rl_WaitForMultipleObjects(count, objects, wait_all, timeout, $)

#define WaitForMultipleObjectsEx(count, objects, wait_all, timeout, alertable)] \
 rl::rl_WaitForMultipleObjectsEx(count, objects, wait_all, timeout, alertable, $)

#define SignalObjectAndWait(obj_to_signal, obj_to_wait, timeout, alertable) \
 rl::rl_SignalObjectAndWait(obj_to_signal, obj_to_wait, timeout, alertable, $)

#ifdef CreateSemaphore
#   undef CreateSemaphore
#endif

#ifdef CreateSemaphore
#   undef ReleaseSemaphore
#endif

#define CreateSemaphoreA rl_CreateSemaphoreMacro
#define CreateSemaphoreW rl_CreateSemaphoreMacro
#define CreateSemaphore rl_CreateSemaphoreMacro
#define rl_CreateSemaphoreMacro(security, initial_count, max_count, name) \
    rl::rl_CreateSemaphore(security, initial_count, max_count, name, $)\

#define ReleaseSemaphore(sema, count, prev_count) \
 rl::rl_ReleaseSemaphore(sema, count, prev_count, $)



#ifdef CreateEvent
#   undef CreateEvent
#endif
#define CreateEventA rl_CreateEventMacro
#define CreateEventW rl_CreateEventMacro
#define CreateEvent rl_CreateEventMacro
#define rl_CreateEventMacro(security, manual_reset, initial_state, name)\
    rl::rl_CreateEvent(security, manual_reset, initial_state, name, $)

#define SetEvent(ev)\
 rl::rl_SetEvent(ev, $)

#define ResetEvent(ev)\
 rl::rl_ResetEvent(ev, $)

#define PulseEvent(ev)\
 rl::rl_PulseEvent(ev, $)


#ifdef CreateMutex
#   undef CreateMutex
#endif
#define CreateMutexA rl_CreateMutexMacro
#define CreateMutexW rl_CreateMutexMacro
#define CreateMutex rl_CreateMutexMacro
#define rl_CreateMutexMacro(security, initial_owner, name)\
    rl::rl_CreateMutex(security, initial_owner, name, $)

#define ReleaseMutex(mtx)\
 rl::rl_ReleaseMutex(mtx, $)



#define CRITICAL_SECTION rl::rl_CRITICAL_SECTION

#define InitializeCriticalSection(cs) \
 rl::rl_InitializeCriticalSection(cs, $)

#define InitializeCriticalSectionAndSpinCount(cs, spin) \
 rl::rl_InitializeCriticalSectionAndSpinCount(cs, spin, $)

#define InitializeCriticalSectionEx(cs, spin, flags) \
 rl::rl_InitializeCriticalSectionEx(cs, spin, flags, $)

#define DeleteCriticalSection(cs) \
 rl::rl_DeleteCriticalSection(cs, $)

#define EnterCriticalSection(cs) \
 rl::rl_EnterCriticalSection(cs, $)

#define TryEnterCriticalSection(cs) \
 rl::rl_TryEnterCriticalSection(cs, $)

#define LeaveCriticalSection(cs) \
 rl::rl_LeaveCriticalSection(cs, $)




#define SRWLOCK rl::rl_SRWLOCK

#define InitializeSRWLock(lock) \
 rl::rl_InitializeSRWLock(lock, $)

#define AcquireSRWLockExclusive(lock) \
 rl::rl_AcquireSRWLockExclusive(lock, $)

#define AcquireSRWLockShared(lock) \
 rl::rl_AcquireSRWLockShared(lock, $)

#define ReleaseSRWLockExclusive(lock) \
 rl::rl_ReleaseSRWLockExclusive(lock, $)

#define ReleaseSRWLockShared(lock) \
 rl::rl_ReleaseSRWLockShared(lock, $)

//!!! no such function in WIN API
#define DeleteSRWLock(lock) \
 rl::rl_DeleteSRWLock(lock, $)






#define CONDITION_VARIABLE rl::rl_CONDITION_VARIABLE

#ifdef CONDITION_VARIABLE_LOCKMODE_SHARED
#   undef CONDITION_VARIABLE_LOCKMODE_SHARED
#endif
#define CONDITION_VARIABLE_LOCKMODE_SHARED rl::rl_CONDITION_VARIABLE_LOCKMODE_SHARED

#define InitializeConditionVariable(cv) \
 rl::rl_InitializeConditionVariable(cv, $)

#define SleepConditionVariableCS(cv, cs, ms) \
 rl::rl_SleepConditionVariableCS(cv, cs, ms, $)

#define SleepConditionVariableSRW(cv, lock, ms, flags) \
 rl::rl_SleepConditionVariableSRW(cv, lock, ms, flags, $)

#define WakeAllConditionVariable(cv) \
 rl::rl_WakeAllConditionVariable(cv, $)

#define WakeConditionVariable(cv) \
 rl::rl_WakeConditionVariable(cv, $)

//!!! no such function in WIN API
#define DeleteConditionVariable(cv) \
 rl::rl_DeleteConditionVariable(cv, $)



#define CreateThread(security, stack_size, fn, param, creation_flags, thread_id) \
 rl::rl_CreateThread(security, stack_size, fn, param, creation_flags, thread_id, $)

#define _beginthreadex(security, stack_size, start_address, arglist, initflag, thrdaddr) \
  rl::rl_beginthreadex(security, stack_size, start_address, arglist, initflag, thrdaddr, $)

#define SetThreadAffinityMask(th, affinity_mask) \
 rl::rl_SetThreadAffinityMask(th, affinity_mask, $)

#define SuspendThread(th) \
 rl::rl_SuspendThread(th, $)

#define ResumeThread(th) \
 rl::rl_ResumeThread(th, $)

#define FlushProcessWriteBuffers() \
 rl::rl_FlushProcessWriteBuffers($)
