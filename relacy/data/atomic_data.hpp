/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#pragma once

#include "../defs.hpp"
#include "../sync_var.hpp"
#include "../waitset.hpp"

namespace rl
{

struct atomic_data {};

struct atomic_data_impl : atomic_data
{
    struct history_record
    {
        history_record(thread_id_t thread_count);

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

    atomic_data_impl(thread_id_t thread_count);
};

}
