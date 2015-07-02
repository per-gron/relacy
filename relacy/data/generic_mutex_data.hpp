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

namespace rl
{

struct generic_mutex_data
{
    generic_mutex_data() = default;
    generic_mutex_data(const generic_mutex_data &) = delete;
    generic_mutex_data &operator=(const generic_mutex_data &) = delete;

    virtual bool lock_exclusive(bool is_timed, debug_info_param info) = 0;
    virtual bool try_lock_exclusive(debug_info_param info) = 0;
    virtual void unlock_exclusive(debug_info_param info) = 0;
    virtual void lock_shared(debug_info_param info) = 0;
    virtual bool try_lock_shared(debug_info_param info) = 0;
    virtual void unlock_shared(debug_info_param info) = 0;
    virtual void unlock_exclusive_or_shared(debug_info_param info) = 0;
    virtual bool is_signaled(debug_info_param info) = 0;
    virtual void memory_acquire(debug_info_param info) = 0;
    virtual void* prepare_wait(debug_info_param info) = 0;
    virtual ~generic_mutex_data() {} // just to calm down gcc
};

}
