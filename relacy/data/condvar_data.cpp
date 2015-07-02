/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "condvar_data.hpp"

namespace rl
{

condvar_data_impl::condvar_data_impl(thread_id_t thread_count, bool allow_spurious_wakeups)
    : ws_(thread_count)
{
    spurious_wakeup_limit_ = 0;
    if (allow_spurious_wakeups && ctx().is_random_sched())
        spurious_wakeup_limit_ = 10;
}

condvar_data_impl::~condvar_data_impl()
{
    //!!! detect destoy when there are blocked threads
}

void condvar_data_impl::event_t::output(std::ostream& s) const
{
    s << "<" << std::hex << var_addr_ << std::dec << "> cond_var: ";
    switch (type_)
    {
    case type_notify_one:
        s << "notify one total_blocked=" << thread_count_ << " unblocked=" << (thread_count_ ? 1 : 0);
        break;
    case type_notify_all:
        s << "notify all unblocked=" << thread_count_;
        break;
    case type_wait_enter: s << "wait enter"; break;
    case type_wait_exit:
        s << "wait exit";
        if (unpark_reason_normal == reason_)
            s << " due to notified";
        else if (unpark_reason_timeout == reason_)
            s << " due to timeout";
        else if (unpark_reason_spurious == reason_)
            s << " spuriously";
        break;
    case type_wait_pred_enter: s << "wait pred enter"; break;
    case type_wait_pred_exit: s << "wait pred exit"; break;
    }
}

void condvar_data_impl::notify_one(debug_info_param info)
{
    context& c = ctx();
    //??? do I need this scheduler call?
    c.sched();
    sign_.check(info);
    RL_HIST(event_t) {this, event_t::type_notify_one, ws_.size()} RL_HIST_END();
    ws_.unpark_one(c, info);
}

void condvar_data_impl::notify_all(debug_info_param info)
{
    context& c = ctx();
    //??? do I need this scheduler call?
    c.sched();
    sign_.check(info);
    RL_HIST(event_t) {this, event_t::type_notify_all, ws_.size()} RL_HIST_END();
    ws_.unpark_all(c, info);
}

sema_wakeup_reason condvar_data_impl::wait(mutex_wrapper const& lock, bool is_timed, debug_info_param info)
{
    //!!! detect whether mutex is the same
    context& c = ctx();
    sign_.check(info);
    RL_HIST(event_t) {this, event_t::type_wait_enter} RL_HIST_END();
    lock.unlock(info);
    sign_.check(info);
    bool allow_spurious_wakeup = (spurious_wakeup_limit_ > 0);
    unpark_reason reason = ws_.park_current(c, is_timed, allow_spurious_wakeup, false, info);
    if (reason == unpark_reason_spurious)
        spurious_wakeup_limit_ -= 1;
    RL_HIST(event_t) {this, event_t::type_wait_exit, 0, reason} RL_HIST_END();
    lock.lock(info);
    sign_.check(info);
    if (reason == unpark_reason_normal)
        return sema_wakeup_reason_success;
    else if (reason == unpark_reason_spurious)
        return sema_wakeup_reason_spurious;
    else //if (reason == unpark_reason_timeout)
        return sema_wakeup_reason_timeout;
}

bool condvar_data_impl::wait(mutex_wrapper const& lock, std::function<bool ()> const& pred, bool is_timed, debug_info_param info)
{
    context& c = ctx();
    sign_.check(info);
    RL_HIST(event_t) {this, event_t::type_wait_pred_enter} RL_HIST_END();
    while (!pred())
    {
        sema_wakeup_reason reason = wait(lock, is_timed, info);
        if (reason == sema_wakeup_reason_timeout)
        {
            RL_HIST(event_t) {this, event_t::type_wait_pred_exit} RL_HIST_END();
            return pred();
        }
    }
    RL_HIST(event_t) {this, event_t::type_wait_pred_exit} RL_HIST_END();
    return true;
}

}
