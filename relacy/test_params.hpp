/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#pragma once

#include <iostream>

#include "base.hpp"
#include "test_result.hpp"

namespace rl
{

enum scheduler_type_e
{
    sched_random,
    sched_bound,
    sched_full,
    sched_count,

    random_scheduler_type = sched_random,
    fair_context_bound_scheduler_type = sched_bound,
    fair_full_search_scheduler_type = sched_full,
    scheduler_type_count
};

char const* format(scheduler_type_e t);

struct test_params
{
    // input params
    iteration_t                 iteration_count;
    std::ostream*               output_stream;
    std::ostream*               progress_stream;
    unsigned                    progress_output_period;
    bool                        collect_history;
    bool                        output_history;
    scheduler_type_e            search_type;
    unsigned                    context_bound;
    unsigned                    execution_depth_limit;
    string                      initial_state;

    // output params
    test_result_e               test_result;
    iteration_t                 stop_iteration;
    string                      test_name;
    string                      final_state;

    test_params();
};

}
