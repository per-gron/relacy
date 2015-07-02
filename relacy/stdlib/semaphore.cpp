/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "semaphore.hpp"

namespace rl
{

void wfmo_event::output(std::ostream& s) const
{
    s   << "WFMO: "
        << "count=" << count_
        << ", wait_all=" << wait_all_
        << ", try_wait=" << try_wait_
        << ", is_timed=" << is_timed_
        << ", result=";
    if (sema_wakeup_reason_success == result_)
    {
        s << "success";
        if (wait_all_ == false)
            s << ", object=" << signaled_;
    }
    else
    {
        s << "timeout";
    }
}

sema_wakeup_reason wait_for_multiple_objects(
    size_t& signaled,
    size_t count,
    win_waitable_object** wo,
    bool wait_all,
    bool try_wait,
    bool is_timed,
    debug_info_param info)
{
    context& c = ctx();
    c.sched();

    RL_VERIFY(count <= wfmo_max_objects);
    void* ws [wfmo_max_objects];

    sema_wakeup_reason result = sema_wakeup_reason_failed;
    signaled = 0;

    if (wait_all)
    {
        for (;;)
        {
            unsigned long i = 0;
            for (i = 0; i != count; ++i)
            {
                if (false == wo[i]->is_signaled(info))
                    break;
            }
            if (i == count)
            {
                preemption_disabler pd (c);
                for (i = 0; i != count; ++i)
                {
                    sema_wakeup_reason r = wo[i]->wait(true, false, info);
                    RL_VERIFY(r == sema_wakeup_reason_success);
                    (void)r;
                }
                result = sema_wakeup_reason_success;
                break;
            }
            else if (try_wait)
            {
                for (i = 0; i != count; ++i)
                    wo[i]->memory_acquire(info);
                result = sema_wakeup_reason_timeout;
                break;
            }
            else
            {
                for (i = 0; i != count; ++i)
                {
                    ws[i] = wo[i]->prepare_wait(info);
                }
                unpark_reason reason = c.wfmo_park(ws, wo, (unsigned)count, !!wait_all, is_timed, info);
                RL_VERIFY(unpark_reason_spurious != reason);
                if (unpark_reason_timeout == reason)
                {
                    for (i = 0; i != count; ++i)
                        wo[i]->memory_acquire(info);
                    result = sema_wakeup_reason_timeout;
                    break;
                }
                else if (unpark_reason_normal == reason)
                {
                    {
                        preemption_disabler pd (c);
                        for (unsigned long i = 0; i != count; ++i)
                        {
                            RL_VERIFY(wo[i]->is_signaled(info));
                            sema_wakeup_reason r = wo[i]->wait(true, false, info);
                            RL_VERIFY(r == sema_wakeup_reason_success);
                            (void)r;
                        }
                    }
                    c.switch_back(info);
                    result = sema_wakeup_reason_success;
                    break;
                }
                RL_VERIFY(false);
            }
        }
    }
    else
    {
        for (;;)
        {
            unsigned long i = 0;
            for (i = 0; i != count; ++i)
            {
                if (true == wo[i]->is_signaled(info))
                    break;
            }
            if (i != count)
            {
                preemption_disabler pd (c);
                sema_wakeup_reason r = wo[i]->wait(true, false, info);
                RL_VERIFY(r == sema_wakeup_reason_success);
                (void)r;
                signaled = i;
                result = sema_wakeup_reason_success;
                break;
            }
            else if (try_wait)
            {
                for (i = 0; i != count; ++i)
                    wo[i]->memory_acquire(info);
                result = sema_wakeup_reason_timeout;
                break;
            }
            else
            {
                for (i = 0; i != count; ++i)
                {
                    ws[i] = wo[i]->prepare_wait(info);
                }
                unpark_reason reason = c.wfmo_park(ws, wo, (unsigned)count, !!wait_all, is_timed, info);
                RL_VERIFY(unpark_reason_spurious != reason);
                if (unpark_reason_timeout == reason)
                {
                    for (i = 0; i != count; ++i)
                        wo[i]->memory_acquire(info);
                    result = sema_wakeup_reason_timeout;
                    break;
                }
                else if (unpark_reason_normal == reason)
                {
                    unsigned long i = 0;
                    for (i = 0; i != count; ++i)
                    {
                        if (true == wo[i]->is_signaled(info))
                            break;
                    }
                    RL_VERIFY(i != count);
                    {
                        preemption_disabler pd (c);
                        sema_wakeup_reason r = wo[i]->wait(true, false, info);
                        RL_VERIFY(r == sema_wakeup_reason_success);
                        (void)r;
                    }
                    c.switch_back(info);
                    signaled = i;
                    result = sema_wakeup_reason_success;
                    break;
                }
                RL_VERIFY(false);
            }
        }
    }

    RL_HIST(wfmo_event) {(unsigned)count, wait_all, try_wait, is_timed, result, signaled} RL_HIST_END();
    return result;
}

}
