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
#include "../signature.hpp"
#include "../sync_var.hpp"
#include "../waitset.hpp"

namespace rl
{

class generic_mutex_data
{
public:
    struct event_t
    {
        enum type_e
        {
            type_lock,
            type_unlock,
            type_recursive_lock,
            type_recursive_unlock,
            type_failed_try_lock,
            type_spuriously_failed_try_lock,
            type_lock_shared,
            type_unlock_shared,
            type_recursive_lock_shared,
            type_recursive_unlock_shared,
            type_failed_try_lock_shared,
            type_spuriously_failed_try_lock_shared,
            type_wait,
            type_destroying_owned_mutex,
        };

        generic_mutex_data const* var_addr_;
        type_e type_;

        void output(std::ostream& s) const;
    };

    generic_mutex_data(thread_id_t thread_count, bool is_rw, bool is_exclusive_recursive, bool is_shared_recursive, bool failing_try_lock);

    generic_mutex_data(const generic_mutex_data &) = delete;
    generic_mutex_data &operator=(const generic_mutex_data &) = delete;

    ~generic_mutex_data();

    bool lock_exclusive(bool is_timed, debug_info_param info);

    bool try_lock_exclusive(debug_info_param info);

    void unlock_exclusive(debug_info_param info);

    void lock_shared(debug_info_param info);

    bool try_lock_shared(debug_info_param info);

    void unlock_shared(debug_info_param info);

    void unlock_exclusive_or_shared(debug_info_param info);

    bool is_signaled(debug_info_param info);

    void memory_acquire(debug_info_param info);

    void* prepare_wait(debug_info_param info);

private:
    static thread_id_t const state_shared = (thread_id_t)-1;
    static thread_id_t const state_free = (thread_id_t)-2;

    signature<0xbabaf1f1> sign_;
    bool is_rw_;
    bool is_exclusive_recursive_;
    bool is_shared_recursive_;
    bool failing_try_lock_;
    sync_var sync_;
    thread_id_t exclusive_owner_;
    unsigned exclusive_recursion_count_;
    waitset exclusive_waitset_;
    waitset shared_waitset_;
    rl_vector<timestamp_t> shared_owner_;
    unsigned shared_lock_count_;
    bool try_lock_failed_;
};

}
