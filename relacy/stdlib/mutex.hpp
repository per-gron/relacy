/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#pragma once

#include "../atomic.hpp"
#include "../base.hpp"
#include "../context.hpp"
#include "../data/thread_info.hpp"
#include "../foreach.hpp"
#include "../generic_mutex_data.hpp"
#include "../signature.hpp"
#include "../sync_var.hpp"
#include "../waitset.hpp"
#include "semaphore.hpp"



namespace rl
{

template<typename type>
class generic_mutex : public win_waitable_object
{
public:
    generic_mutex()
        : impl_()
    {
    }

    generic_mutex(generic_mutex const&)
        : impl_()
    {
    }

    generic_mutex& operator = (generic_mutex const&)
    {
        return *this;
    }

    ~generic_mutex()
    {
    }

    void init(bool is_rw, bool is_exclusive_recursive, bool is_shared_recursive, bool failing_try_lock, debug_info_param info)
    {
        context& c = ctx();
        RL_ASSERT_IMPL(0 == impl_, test_result_double_initialization_of_mutex, "", info);
        sign_.check(info);
        impl_ = c.mutex_ctor(is_rw, is_exclusive_recursive, is_shared_recursive, failing_try_lock);
    }

    void deinit(debug_info_param info)
    {
        context& c = ctx();
        check(info);
        c.mutex_dtor(impl_);
        impl_ = 0;
    }

    void lock(debug_info_param info)
    {
        lock_exclusive(info);
    }

    bool lock_exclusive_timed(debug_info_param info)
    {
        return check(info)->lock_exclusive(true, info);
    }

    void unlock(debug_info_param info)
    {
        unlock_exclusive(info);
    }

    void lock_exclusive(debug_info_param info)
    {
        check(info)->lock_exclusive(false, info);
    }

    bool try_lock_exclusive(debug_info_param info)
    {
        return check(info)->try_lock_exclusive(info);
    }

    void unlock_exclusive(debug_info_param info)
    {
        check(info)->unlock_exclusive(info);
    }

    void lock_shared(debug_info_param info)
    {
        check(info)->lock_shared(info);
    }

    bool try_lock_shared(debug_info_param info)
    {
        return check(info)->try_lock_shared(info);
    }

    void unlock_shared(debug_info_param info)
    {
        check(info)->unlock_shared(info);
    }

    void unlock_exclusive_or_shared(debug_info_param info)
    {
        check(info)->unlock_exclusive_or_shared(info);
    }

private:
    generic_mutex_data* impl_;
    signature<0x6A6cB03A> sign_;

    generic_mutex_data* check(debug_info_param info)
    {
        RL_ASSERT_IMPL(impl_, test_result_usage_of_non_initialized_mutex, "", info);
        sign_.check(info);
        return impl_;
    }

    virtual sema_wakeup_reason wait(bool try_wait, bool is_timed, debug_info_param info)
    {
        if (try_wait)
        {
            if (check(info)->try_lock_exclusive(info))
                return sema_wakeup_reason_success;
            else
                return sema_wakeup_reason_failed;
        }
        else
        {
            if (check(info)->lock_exclusive(is_timed, info))
                return sema_wakeup_reason_success;
            else
                return sema_wakeup_reason_timeout;

        }
    }

    virtual bool signal(debug_info_param info)
    {
        check(info)->unlock_exclusive(info);
        return true;
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




template<typename tag, bool is_recursive>
class std_generic_mutex : generic_mutex<tag>
{
public:
    std_generic_mutex()
    {
        generic_mutex<tag>::init(false, is_recursive, false, true, $);
    }

    std_generic_mutex(const std_generic_mutex &) = delete;
    std_generic_mutex &operator=(const std_generic_mutex &) = delete;

    ~std_generic_mutex()
    {
        generic_mutex<tag>::deinit($);
    }

    void lock(debug_info_param info)
    {
        generic_mutex<tag>::lock_exclusive(info);
    }

    bool try_lock(debug_info_param info)
    {
        return generic_mutex<tag>::try_lock_exclusive(info);
    }

    void unlock(debug_info_param info)
    {
        generic_mutex<tag>::unlock_exclusive(info);
    }
};


struct mutex_tag_std;
typedef std_generic_mutex<mutex_tag_std, false> mutex;

struct mutex_tag_std_recursive;
typedef std_generic_mutex<mutex_tag_std_recursive, true> recursive_mutex;


}
