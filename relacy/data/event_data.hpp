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
#include "../stdlib/semaphore.hpp"
#include "../sync_var.hpp"
#include "../waitset.hpp"

namespace rl
{

class event_data
{
public:
    event_data(thread_id_t thread_count, bool manual_reset, bool initial_state);

    event_data(const event_data &) = delete;
    event_data &operator=(const event_data &) = delete;

    ~event_data();

    void set(debug_info_param info);

    void reset(debug_info_param info);

    void pulse(debug_info_param info);

    sema_wakeup_reason wait(bool try_wait, bool is_timed, debug_info_param info);

    bool is_signaled(debug_info_param info);

    void memory_acquire(debug_info_param info);

    void* prepare_wait(debug_info_param info);

private:
    signature<0xdada1234> sign_;
    bool const manual_reset_;
    bool state_;
    waitset ws_;
    sync_var sync_;

    struct state_event
    {
        enum type
        {
            type_set,
            type_reset,
            type_pulse,
        };

        event_data* addr_;
        type type_;
        bool initial_state_;
        bool final_state_;
        thread_id_t unblocked_;

        void output(std::ostream& s) const;
    };

    struct wait_event
    {
        event_data* addr_;
        bool try_wait_;
        bool is_timed_;
        bool initial_state_;
        bool final_state_;
        sema_wakeup_reason reason_;

        void output(std::ostream& s) const;
    };
};

}
