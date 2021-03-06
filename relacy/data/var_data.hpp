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

namespace rl
{

struct var_data
{
    rl_vector<timestamp_t> load_acq_rel_timestamp_;
    rl_vector<timestamp_t> store_acq_rel_timestamp_;

    var_data(thread_id_t thread_count);

    void init(thread_info& th);

    bool store(thread_info& th);

    bool load(thread_info& th);
};

}
