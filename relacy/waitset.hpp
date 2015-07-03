/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#pragma once

#include "base.hpp"
#include "defs.hpp"

namespace rl
{

class context;
struct thread_info;

class waitset
{
public:
    waitset(thread_id_t thread_count);

    unpark_reason park_current(context& c,
                               bool is_timed,
                               bool allow_spurious_wakeup,
                               bool do_switch,
                               debug_info_param info);

    static unpark_reason park_current(context& c,
                                      waitset** ws,
                                      win_waitable_object** wo,
                                      size_t count,
                                      bool wait_all,
                                      bool is_timed,
                                      bool do_switch,
                                      debug_info_param info);

    bool unpark_one(context& c, debug_info_param info);

    thread_id_t unpark_all(context& c, debug_info_param info);

    thread_id_t size() const;

    operator bool () const;

private:
    struct thread_desc
    {
        thread_info*            th_;
        unsigned                count_;     // 0 - wfso, !0 - wfmo
        waitset**               ws_;        // 0 - wfso, !0 - wfmo
        win_waitable_object**   wo_;        // 0 - wfso, !0 - wfmo
        bool                    wait_all_;
        bool                    do_switch_;
    };

    rl_vector<thread_desc>      set_;
    thread_id_t                 size_;

    bool try_remove(context& c, thread_id_t const idx, debug_info_param info);

    void remove(thread_info* th);

    static void remove(thread_info* th, waitset** ws, unsigned count);
};

}
