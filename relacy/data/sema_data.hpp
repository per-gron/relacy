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
#include "../sync_var.hpp"
#include "../waitset.hpp"
#include "../signature.hpp"

namespace rl
{

enum sema_wakeup_reason
{
    sema_wakeup_reason_success,
    sema_wakeup_reason_failed,
    sema_wakeup_reason_timeout,
    sema_wakeup_reason_spurious,
};

class sema_data
{
public:
    sema_data(thread_id_t thread_count,
                   bool spurious_wakeups,
                   unsigned initial_count,
                   unsigned max_count);

    sema_data(const sema_data &) = delete;
    sema_data &operator=(const sema_data &) = delete;

    ~sema_data();

    struct wait_event
    {
        sema_data*         addr_;
        bool                    try_wait_;
        bool                    is_timed_;
        unsigned                count_;
        sema_wakeup_reason      reason_;

        void output(std::ostream& s) const;
    };

    struct post_event
    {
        sema_data*         addr_;
        unsigned                value_;
        unsigned                count_;
        bool                    result_;
        thread_id_t             unblocked_;

        void output(std::ostream& s) const;
    };

    struct get_value_event
    {
        sema_data* addr_;
        unsigned count_;

        void output(std::ostream& s) const;
    };

    sema_wakeup_reason wait(bool try_wait,
                            bool is_timed,
                            debug_info_param info);

    bool post(unsigned count, unsigned& prev_count, debug_info_param info);

    int get_value(debug_info_param info);

    bool is_signaled(debug_info_param info);

    void memory_acquire(debug_info_param info);

    void* prepare_wait(debug_info_param info);

private:
    signature<0xaabb6634> sign_;
    bool const spurious_wakeups_;
    unsigned count_;
    unsigned const max_count_;
    waitset ws_;
    sync_var sync_;
};

}
