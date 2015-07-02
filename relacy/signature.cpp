/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "signature.hpp"
#include "context_base.hpp"

namespace rl
{

namespace detail
{

struct fault_event
{
    void const* addr_;
    void output(std::ostream& s) const
    {
        s << "<" << std::hex << addr_ << std::dec << ">"
            << " access to freed memory";
    }
};

RL_NOINLINE void fail(const void *address, debug_info_param info)
{
    context& c = ctx();
    RL_HIST(fault_event) {address} RL_HIST_END();
    rl::ctx().fail_test("access to freed memory", test_result_access_to_freed_memory, info);
}

}

}
