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
#include "waitset.hpp"
#include "sync_var.hpp"
#include "stdlib/semaphore.hpp"


namespace rl
{

class thread_sync_object : public win_waitable_object
{
public:
    thread_sync_object(thread_id_t thread_count);

    void iteration_begin();

    void on_create();

    void on_start();

    void on_finish();

private:
    bool finished_;
    waitset ws_;
    sync_var sync_;

    virtual void deinit(debug_info_param info);

    virtual sema_wakeup_reason wait(bool try_wait, bool is_timed, debug_info_param info);

    virtual bool signal(debug_info_param info);

    virtual bool is_signaled(debug_info_param info);

    virtual void memory_acquire(debug_info_param info);

    virtual void* prepare_wait(debug_info_param info);
};


}
