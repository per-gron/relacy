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

namespace rl
{

class thread_info;

class sync_var
{
public:
    sync_var(thread_id_t thread_count);

    sync_var(const sync_var &) = delete;
    sync_var &operator=(const sync_var &) = delete;

    void iteration_begin();

    void acquire(thread_info* th);

    void release(thread_info* th);

    void acq_rel(thread_info* th);

private:
    rl_vector<timestamp_t> order_;
};

}
