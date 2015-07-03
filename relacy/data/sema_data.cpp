/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "sema_data.hpp"

#include "../context_base.hpp"

namespace rl
{

sema_data_impl::sema_data_impl(
    thread_id_t thread_count,
    bool spurious_wakeups,
    unsigned initial_count,
    unsigned max_count)
    : spurious_wakeups_(spurious_wakeups)
    , count_(initial_count)
    , max_count_(max_count)
    , ws_(thread_count)
    , sync_(thread_count)
{
    RL_VERIFY(max_count <= INT_MAX);
}

sema_data_impl::~sema_data_impl()
{
    //!!! detect destruction with waiters
}

void sema_data_impl::wait_event::output(std::ostream& s) const
{
    s << "<" << std::hex << addr_ << std::dec << "> semaphore: ";
    if (try_wait_)
        s << "try_wait ";
    else if (is_timed_)
        s << "timed wait ";
    else
        s << "wait ";

    if (reason_ == sema_wakeup_reason_success)
        s << "succeeded ";
    else if (reason_ == sema_wakeup_reason_failed)
        s << "failed ";
    else if (reason_ == sema_wakeup_reason_timeout)
        s << "timed out ";
    else if (reason_ == sema_wakeup_reason_spurious)
        s << "spuriously failed ";

    s << "new_count=" << count_;
}

void sema_data_impl::post_event::output(std::ostream& s) const
{
    s << "<" << std::hex << addr_ << std::dec << "> semaphore: ";
    if (result_)
        s << "post ";
    else
        s << "post FAILED ";

    s << "value=" << value_;
    s << " new_count=" << count_;
    s << " unblocked=" << unblocked_;
}

void sema_data_impl::get_value_event::output(std::ostream& s) const
{
    s << "<" << std::hex << addr_ << std::dec << "> semaphore: ";
    s << "get_value count=" << count_;
}

sema_wakeup_reason sema_data_impl::wait(
    bool try_wait,
    bool is_timed,
    debug_info_param info)
{
    context& c = ctx();
    c.sched();
    sign_.check(info);

    sema_wakeup_reason reason = sema_wakeup_reason_success;
    for (;;)
    {
        if (count_)
        {
            count_ -= 1;
            sync_.acq_rel(c.threadx_);
            reason = sema_wakeup_reason_success;
            break;
        }

        if (try_wait)
        {
            sync_.acquire(c.threadx_);
            reason = sema_wakeup_reason_failed;
            break;
        }

        unpark_reason wr = ws_.park_current(c, is_timed, spurious_wakeups_, true, info);
        if (unpark_reason_timeout == wr)
        {
            RL_VERIFY(is_timed);
            sync_.acquire(c.threadx_);
            reason = sema_wakeup_reason_timeout;
            break;
        }
        else if (unpark_reason_spurious == wr)
        {
            RL_VERIFY(spurious_wakeups_);
            sync_.acquire(c.threadx_);
            reason = sema_wakeup_reason_spurious;
            break;
        }
        else if (unpark_reason_normal == wr)
        {
            RL_VERIFY(count_ > 0);
            count_ -= 1;
            sync_.acq_rel(c.threadx_);
            c.switch_back(info);
            reason = sema_wakeup_reason_success;
            break;
        }
        RL_VERIFY(false);
    }

    RL_HIST(wait_event) {this, try_wait, is_timed, count_, reason} RL_HIST_END();
    return reason;
}

bool sema_data_impl::post(unsigned count, unsigned& prev_count, debug_info_param info)
{
    context& c = ctx();
    c.sched();
    sign_.check(info);

    bool result = false;
    prev_count = count_;
    thread_id_t unblocked = 0;
    if (false == (count >= INT_MAX || count + count_ > max_count_))
    {
        result = true;
        count_ += count;
        sync_.acq_rel(c.threadx_);
        for (unsigned i = 0; i != count; ++i)
        {
            if (false == ws_.unpark_one(c, info))
                break;
            unblocked += 1;
        }
    }
    else
    {
        sync_.acquire(c.threadx_);
    }
    RL_HIST(post_event) {this, count, count_, result, unblocked} RL_HIST_END();
    return result;
}

int sema_data_impl::get_value(debug_info_param info)
{
    context& c = ctx();
    c.sched();
    sign_.check(info);

    RL_VERIFY(count_ <= INT_MAX);
    int result = (int)count_ - ws_.size();
    sync_.acquire(c.threadx_);

    RL_HIST(get_value_event) {this, (unsigned)result} RL_HIST_END();
    return result;
}

bool sema_data_impl::is_signaled(debug_info_param info)
{
    (void)info;
    return count_ > 0;
}

void sema_data_impl::memory_acquire(debug_info_param info)
{
    (void)info;
    sync_.acquire(ctx().threadx_);
}

void* sema_data_impl::prepare_wait(debug_info_param info)
{
    (void)info;
    return &ws_;
}

}
