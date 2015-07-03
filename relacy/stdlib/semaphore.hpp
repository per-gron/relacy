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
#include "../data/sema_data.hpp"
#include "../sync_var.hpp"
#include "../waitset.hpp"
#include "../signature.hpp"
#include "waitable_object.hpp"


namespace rl
{

template<typename tag_t>
class semaphore : public win_waitable_object
{
public:
    semaphore()
        : impl_()
    {
    }

    semaphore(semaphore const&)
        : impl_()
    {
    }

    semaphore& operator = (semaphore const&)
    {
        return *this;
    }

    void init(bool spurious_wakeups, unsigned initial_count, unsigned max_count, debug_info_param info)
    {
        context& c = ctx();
        RL_ASSERT_IMPL(0 == impl_, test_result_double_initialization_of_semaphore, "", info);
        sign_.check(info);
        impl_ = c.sema_ctor(spurious_wakeups, initial_count, max_count);
    }

    void deinit(debug_info_param info)
    {
        context& c = ctx();
        check(info);
        c.sema_dtor(impl_);
        impl_ = 0;
    }

    virtual sema_wakeup_reason wait(bool try_wait, bool is_timed, debug_info_param info)
    {
        check(info);
        return impl_->wait(try_wait, is_timed, info);
    }

    virtual bool signal(debug_info_param info)
    {
        unsigned prev_count = 0;
        return post(1, prev_count, info);
    }

    bool post(unsigned count, unsigned& prev_count, debug_info_param info)
    {
        check(info);
        return impl_->post(count, prev_count, info);
    }

    int get_value(debug_info_param info)
    {
        check(info);
        return impl_->get_value(info);
    }

private:
    sema_data* impl_;
    signature<0x228855dd> sign_;

    sema_data* check(debug_info_param info)
    {
        RL_ASSERT_IMPL(impl_, test_result_usage_of_non_initialized_semaphore, "", info);
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



struct wfmo_event
{
    unsigned long               count_;
    bool                        wait_all_;
    bool                        try_wait_;
    bool                        is_timed_;
    sema_wakeup_reason          result_;
    size_t                      signaled_;

    void output(std::ostream& s) const;
};

size_t const wfmo_max_objects = 32;

sema_wakeup_reason wait_for_multiple_objects(
    size_t& signaled,
    size_t count,
    win_waitable_object** wo,
    bool wait_all,
    bool try_wait,
    bool is_timed,
    debug_info_param info);

}
