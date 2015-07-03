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


namespace rl
{

struct win_object
{
    virtual void deinit(debug_info_param info) = 0;
    virtual ~win_object() {}
};

struct win_waitable_object : win_object
{
    virtual sema_wakeup_reason wait(bool try_wait, bool is_timed, debug_info_param info) = 0;
    virtual bool signal(debug_info_param info) = 0;

    virtual bool is_signaled(debug_info_param info) = 0;
    virtual void memory_acquire(debug_info_param info) = 0;
    virtual void* prepare_wait(debug_info_param info) = 0;
};

}
