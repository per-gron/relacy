/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#pragma once

#include "atomic_data.hpp"

namespace rl
{

atomic_data_impl::history_record::history_record(thread_id_t thread_count)
    : acq_rel_order_(thread_count)
    , last_seen_order_(thread_count) {}

atomic_data_impl::atomic_data_impl(thread_id_t thread_count)
    : futex_ws_(thread_count)
    , futex_sync_(thread_count)
{
    history_.reserve(history_size);
    for (thread_id_t i = 0; i < history_size; i++) {
        history_.emplace_back(thread_count);
    }

    current_index_ = 0;
    history_record& rec = history_[0];
    history_[atomic_history_size - 1].busy_ = false;

    rec.busy_ = false;
    rec.seq_cst_ = false;
    rec.thread_id_ = (thread_id_t)-1;
}

}
