/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#pragma once

#include "../base.hpp"
#include "../mutex_wrapper.hpp"
#include "../signature.hpp"
#include "../stdlib/semaphore.hpp"
#include "../waitset.hpp"

namespace rl
{

struct condvar_data
{
    virtual void notify_one(debug_info_param info) = 0;
    virtual void notify_all(debug_info_param info) = 0;
    virtual sema_wakeup_reason wait(mutex_wrapper const& lock, bool is_timed, debug_info_param info) = 0;
    virtual bool wait(mutex_wrapper const& lock, std::function<bool ()> const& pred, bool is_timed, debug_info_param info) = 0;
    virtual ~condvar_data() {} // just to calm down gcc
};

class condvar_data_impl : public condvar_data
{
public:
    condvar_data_impl(thread_id_t thread_count, bool allow_spurious_wakeups)
        : ws_(thread_count)
    {
        spurious_wakeup_limit_ = 0;
        if (allow_spurious_wakeups && ctx().is_random_sched())
            spurious_wakeup_limit_ = 10;
    }

    ~condvar_data_impl()
    {
        //!!! detect destoy when there are blocked threads
    }

private:
    waitset                ws_;
    signature<0xc0ffe3ad>  sign_;
    int                    spurious_wakeup_limit_;

    struct event_t
    {
        enum type_e
        {
            type_notify_one,
            type_notify_all,
            type_wait_enter,
            type_wait_exit,
            type_wait_pred_enter,
            type_wait_pred_exit,
        };

        condvar_data_impl const*    var_addr_;
        type_e                      type_;
        thread_id_t                 thread_count_;
        unpark_reason               reason_;

        void output(std::ostream& s) const
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
    };

    virtual void notify_one(debug_info_param info)
    {
        context& c = ctx();
        //??? do I need this scheduler call?
        c.sched();
        sign_.check(info);
        RL_HIST(event_t) {this, event_t::type_notify_one, ws_.size()} RL_HIST_END();
        ws_.unpark_one(c, info);
    }

    virtual void notify_all(debug_info_param info)
    {
        context& c = ctx();
        //??? do I need this scheduler call?
        c.sched();
        sign_.check(info);
        RL_HIST(event_t) {this, event_t::type_notify_all, ws_.size()} RL_HIST_END();
        ws_.unpark_all(c, info);
    }

    virtual sema_wakeup_reason wait(mutex_wrapper const& lock, bool is_timed, debug_info_param info)
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

    virtual bool wait(mutex_wrapper const& lock, std::function<bool ()> const& pred, bool is_timed, debug_info_param info)
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
};

}
