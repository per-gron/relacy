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
#include "../context_base.hpp"
#include "../mutex_wrapper.hpp"
#include "../waitset.hpp"
#include "../signature.hpp"


namespace rl
{

struct condvar_data;

template<typename tag_t>
class condvar
{
public:
    condvar()
        : impl_()
    {
    }

    condvar(condvar const&)
        : impl_()
    {
    }

    condvar& operator = (condvar const&)
    {
        return *this;
    }

    ~condvar()
    {
    }

    void init(bool allow_spurious_wakeups, debug_info_param info)
    {
        context& c = ctx();
        RL_ASSERT_IMPL(0 == impl_, test_result_double_initialization_of_condvar, "", info);
        sign_.check(info);
        impl_ = c.condvar_ctor(allow_spurious_wakeups);
    }

    void deinit(debug_info_param info)
    {
        context& c = ctx();
        check(info);
        c.condvar_dtor(impl_);
        impl_ = 0;
    }

    void notify_one(debug_info_param info)
    {
        check(info);
        impl_->notify_one(info);
    }

    void notify_all(debug_info_param info)
    {
        check(info);
        impl_->notify_all(info);
    }

    template<typename lock_t>
    sema_wakeup_reason wait(lock_t& lock, bool is_timed, debug_info_param info)
    {
        check(info);
        mutex_wrapper_impl<lock_t> w (lock);
        return impl_->wait(w, is_timed, info);
    }

    template<typename lock_t>
    bool wait(mutex_wrapper const& lock, std::function<bool ()> const& pred, bool is_timed, debug_info_param info)
    {
        check(info);
        return impl_->wait(mutex_wrapper_impl<lock_t>(lock), pred, is_timed, info);
    }

private:
    condvar_data* impl_;
    signature<0xbadc0ffe> sign_;

    void check(debug_info_param info)
    {
        RL_ASSERT_IMPL(impl_, test_result_usage_of_non_initialized_condvar, "", info);
        sign_.check(info);
    }
};



template<typename tag_t>
class condition_variable_std : condvar<tag_t>
{
public:
    condition_variable_std()
    {
        condvar<tag_t>::init(true, $);
    }

    condition_variable_std(const condition_variable_std &) = delete;
    condition_variable_std &operator=(const condition_variable_std &) = delete;

    ~condition_variable_std()
    {
        condvar<tag_t>::deinit($);
    }

    void notify_one(debug_info_param info)
    {
        condvar<tag_t>::notify_one(info);
    }

    void notify_all(debug_info_param info)
    {
        condvar<tag_t>::notify_all(info);
    }

    template<typename lock_t>
    void wait(lock_t& lock, debug_info_param info)
    {
        condvar<tag_t>::wait(lock, false, info);
    }

    template<typename lock_t, typename pred_t>
    void wait(lock_t& lock, pred_t pred, debug_info_param info)
    {
        condvar<tag_t>::wait(lock, pred, false, info);
    }

    template<typename lock_t, typename abs_time_t>
    bool wait_until(lock_t& lock, abs_time_t const&, debug_info_param info)
    {
        return condvar<tag_t>::wait(lock, true, info);
    }

    template<typename lock_t, typename abs_time_t, typename pred_t>
    bool wait_until(lock_t& lock, abs_time_t const&, pred_t pred, debug_info_param info)
    {
        return condvar<tag_t>::wait(lock, pred, true, info);
    }

    template<typename lock_t, typename rel_time_t>
    bool wait_for(lock_t& lock, rel_time_t const&, debug_info_param info)
    {
        sema_wakeup_reason reason = condvar<tag_t>::wait(lock, true, info);
        return reason == sema_wakeup_reason_success;
    }

    template<typename lock_t, typename rel_time_t, typename pred_t>
    bool wait_for(lock_t& lock, rel_time_t const&, pred_t pred, debug_info_param info)
    {
        return condvar<tag_t>::wait(lock, pred, true, info);
    }
};


struct condvar_tag_std;
typedef condition_variable_std<condvar_tag_std> condition_variable;
struct condvar_tag_std_any;
typedef condition_variable_std<condvar_tag_std_any> condition_variable_any;

}
