/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "event_data.hpp"

namespace rl
{

event_data_impl::event_data_impl(thread_id_t thread_count, bool manual_reset, bool initial_state)
    : manual_reset_(manual_reset)
    , state_(initial_state)
    , ws_(thread_count)
    , sync_(thread_count)
{
}

event_data_impl::~event_data_impl()
{
    //!!! detect destuction with waiters
}

void event_data_impl::state_event::output(std::ostream& s) const
{
    s << "<" << std::hex << addr_ << std::dec << "> event: ";
    if (type_set == type_)
        s << "set ";
    else if (type_reset == type_)
        s << "reset ";
    else
        s << "pulse ";
    s << "initial_state=" << initial_state_
        << " final_state=" << final_state_;
    if (type_reset != type_)
        s << " unblocked=" << unblocked_;
}

void event_data_impl::set(debug_info_param info)
{
    context& c = ctx();
    c.sched();
    sign_.check(info);

    bool initial_state = state_;
    thread_id_t unblocked = 0;

    if (state_)
    {
        //!!! probably can break if a thread waits in wfmo
        RL_VERIFY(false == ws_);
    }
    else
    {
        sync_.release(c.threadx_);
        state_ = true;

        if (manual_reset_)
        {
            unblocked = ws_.unpark_all(c, info);
        }
        else
        {
            if (ws_.unpark_one(c, info))
                unblocked = 1;
        }
    }

    RL_HIST(state_event) {this, state_event::type_set, initial_state, state_, unblocked} RL_HIST_END();
}

void event_data_impl::reset(debug_info_param info)
{
    context& c = ctx();
    c.sched();
    sign_.check(info);

    bool initial_state = state_;

    if (state_)
    {
        RL_VERIFY(false == ws_);
        sync_.release(c.threadx_);
        state_ = false;
    }

    RL_HIST(state_event) {this, state_event::type_reset, initial_state, state_, 0} RL_HIST_END();
}

void event_data_impl::pulse(debug_info_param info)
{
    context& c = ctx();
    c.sched();
    sign_.check(info);

    //??? should I model nasty caveat described in MSDN
    thread_id_t unblocked = 0;

    if (state_)
    {
        //!!! probably can break if a thread waits in wfmo
        RL_VERIFY(false == ws_);
    }
    else
    {
        sync_.release(c.threadx_);
        state_ = true;
        unblocked = ws_.unpark_all(c, info);
        state_ = false;
    }

    RL_HIST(state_event) {this, state_event::type_pulse, state_, state_, unblocked} RL_HIST_END();
}

void event_data_impl::wait_event::output(std::ostream& s) const
{
    s << "<" << std::hex << addr_ << std::dec << "> event: ";
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

    s << "initial_state=" << initial_state_
        << " final_state=" << final_state_;
}

sema_wakeup_reason event_data_impl::wait(bool try_wait, bool is_timed, debug_info_param info)
{
    context& c = ctx();
    c.sched();
    sign_.check(info);

    bool initial_state = state_;
    sema_wakeup_reason reason = sema_wakeup_reason_success;

    for (;;)
    {
        if (state_)
        {
            if (manual_reset_)
            {
                sync_.acquire(c.threadx_);
            }
            else
            {
                state_ = false;
                sync_.acq_rel(c.threadx_);
            }
            reason = sema_wakeup_reason_success;
            break;
        }

        if (try_wait)
        {
            sync_.acquire(c.threadx_);
            reason = sema_wakeup_reason_failed;
            break;
        }

        unpark_reason wr = ws_.park_current(c, is_timed, false, true, info);
        initial_state = state_;
        if (unpark_reason_timeout == wr)
        {
            sync_.acquire(c.threadx_);
            reason = sema_wakeup_reason_timeout;
            break;
        }
        else if (unpark_reason_normal == wr)
        {
            RL_VERIFY(state_ == true);
            if (manual_reset_)
            {
                sync_.acquire(c.threadx_);
            }
            else
            {
                state_ = false;
                sync_.acq_rel(c.threadx_);
            }
            c.switch_back(info);
            reason = sema_wakeup_reason_success;
            break;
        }
        RL_VERIFY(false);
    }

    RL_HIST(wait_event) {this, try_wait, is_timed, initial_state, state_, reason} RL_HIST_END();
    return reason;
}

bool event_data_impl::is_signaled(debug_info_param info)
{
    (void)info;
    return state_;
}

void event_data_impl::memory_acquire(debug_info_param info)
{
    (void)info;
    sync_.acquire(ctx().threadx_);
}

void* event_data_impl::prepare_wait(debug_info_param info)
{
    (void)info;
    return &ws_;
}

}
