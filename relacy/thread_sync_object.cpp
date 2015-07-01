/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "thread_sync_object.hpp"

namespace rl
{

thread_sync_object::thread_sync_object(thread_id_t thread_count)
    : ws_(thread_count)
    , sync_(thread_count)
{
}

void thread_sync_object::iteration_begin()
{
    finished_ = false;
    sync_.iteration_begin();
    RL_VERIFY(!ws_);
}

void thread_sync_object::on_create()
{
    sync_.release(ctx().threadx_);
}

void thread_sync_object::on_start()
{
    RL_VERIFY(finished_ == false);
    context& c = ctx();
    sync_.acquire(c.threadx_);
}

void thread_sync_object::on_finish()
{
    RL_VERIFY(finished_ == false);
    context& c = ctx();
    finished_ = true;
    sync_.release(c.threadx_);
    ws_.unpark_all(c, $);
}

void thread_sync_object::deinit(debug_info_param info)
{
    (void)info;
}

sema_wakeup_reason thread_sync_object::wait(bool try_wait, bool is_timed, debug_info_param info)
{
    context& c = ctx();
    if (finished_)
    {
        sync_.acquire(c.threadx_);
        return sema_wakeup_reason_success;
    }
    else if (try_wait)
    {
        sync_.acquire(c.threadx_);
        return sema_wakeup_reason_failed;
    }
    else
    {
        unpark_reason reason = ws_.park_current(c, is_timed, false, false, info);
        sync_.acquire(c.threadx_);
        if (reason == unpark_reason_normal)
            return sema_wakeup_reason_success;
        else if (reason == unpark_reason_timeout)
            return sema_wakeup_reason_timeout;
        RL_VERIFY(false);
        return sema_wakeup_reason_failed;
    }
}

bool thread_sync_object::signal(debug_info_param info)
{
    RL_ASSERT_IMPL(false, test_result_thread_signal, "trying to signal a thread", info);
    return false;
}

bool thread_sync_object::is_signaled(debug_info_param info)
{
    (void)info;
    return finished_;
}

void thread_sync_object::memory_acquire(debug_info_param info)
{
    (void)info;
    sync_.acquire(ctx().threadx_);
}

void* thread_sync_object::prepare_wait(debug_info_param info)
{
    (void)info;
    return &ws_;
}

}
