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
#include "../sync_var.hpp"
#include "../waitset.hpp"
#include "semaphore.hpp"


namespace rl
{

struct event_data;

class generic_event : public win_waitable_object
{
public:
    generic_event()
        : impl_()
    {
    }

    generic_event(generic_event const&)
        : impl_()
    {
    }

    generic_event& operator = (generic_event const&)
    {
        return *this;
    }

    void init(bool manual_reset, bool initial_state, debug_info_param info)
    {
        context& c = ctx();
        RL_ASSERT_IMPL(0 == impl_, test_result_double_initialization_of_event, "", info);
        sign_.check(info);
        impl_ = c.event_ctor(manual_reset, initial_state);
    }

    void deinit(debug_info_param info)
    {
        context& c = ctx();
        check(info);
        c.event_dtor(impl_);
        impl_ = 0;
    }

    void set(debug_info_param info)
    {
        check(info);
        impl_->set(info);
    }

    void reset(debug_info_param info)
    {
        check(info);
        impl_->reset(info);
    }

    void pulse(debug_info_param info)
    {
        check(info);
        impl_->pulse(info);
    }

    virtual sema_wakeup_reason wait(bool try_wait, bool is_timed, debug_info_param info)
    {
        check(info);
        return impl_->wait(try_wait, is_timed, info);
    }

    virtual bool signal(debug_info_param info)
    {
        set(info);
        return true;
    }

private:
    event_data* impl_;
    signature<0x3390eeaa> sign_;

    event_data* check(debug_info_param info)
    {
        RL_ASSERT_IMPL(impl_, test_result_usage_of_non_initialized_event, "", info);
        sign_.check(info);
        return impl_;
    }

    virtual bool is_signaled(debug_info_param info)
    {
        return check(info)->is_signaled(info);
    }

    virtual void memory_acquire(debug_info_param info)
    {
        check(info)->memory_acquire(info);
    }

    virtual void* prepare_wait(debug_info_param info)
    {
        return check(info)->prepare_wait(info);
    }
};


}
