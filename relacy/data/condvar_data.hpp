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
    condvar_data_impl(thread_id_t thread_count, bool allow_spurious_wakeups);

    ~condvar_data_impl();

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

        void output(std::ostream& s) const;
    };

    virtual void notify_one(debug_info_param info);

    virtual void notify_all(debug_info_param info);

    virtual sema_wakeup_reason wait(mutex_wrapper const& lock, bool is_timed, debug_info_param info);

    virtual bool wait(mutex_wrapper const& lock, std::function<bool ()> const& pred, bool is_timed, debug_info_param info);
};

}
