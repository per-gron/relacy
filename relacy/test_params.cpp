/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "test_params.hpp"

namespace rl
{

char const* format(scheduler_type_e t)
{
    switch (t)
    {
    case sched_random: return "random scheduler";
    case sched_bound: return "context bound scheduler";
    case sched_full: return "full search scheduler";
    default: break;
    }
    RL_VERIFY(false);
    throw std::logic_error("invalid scheduler type");
}


test_params::test_params()
{
    iteration_count         = 1000;
    output_stream           = &std::cout;
    progress_stream         = &std::cout;
    progress_output_period  = 3;
    collect_history         = false;
    output_history          = false;
    search_type             = random_scheduler_type;
    context_bound           = 1;
    execution_depth_limit   = 2000;

    test_result             = test_result_success;
    stop_iteration          = 0;
}

}
