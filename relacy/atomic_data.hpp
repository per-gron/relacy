/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#pragma once

#include "defs.hpp"
#include "sync_var.hpp"
#include "waitset.hpp"

namespace rl
{

struct atomic_data_impl : atomic_data
{
    struct history_record
    {
        history_record(thread_id_t thread_count)
            : acq_rel_order_(thread_count)
            , last_seen_order_(thread_count) {}

        rl_vector<timestamp_t> acq_rel_order_;
        rl_vector<timestamp_t> last_seen_order_;

        bool busy_;
        bool seq_cst_;
        thread_id_t thread_id_;
        timestamp_t acq_rel_timestamp_;
    };

    static size_t const history_size = atomic_history_size;
    rl_vector<aligned<history_record>> history_;
    unsigned current_index_;
    waitset futex_ws_;
    sync_var futex_sync_;

    atomic_data_impl(thread_id_t thread_count)
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
};

}
