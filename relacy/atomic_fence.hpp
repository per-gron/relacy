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
#include "context.hpp"
#include "memory_order.hpp"


namespace rl
{


struct atomic_fence_event
{
    memory_order mo_;
    bool is_thread_fence_;

    void output(std::ostream& s) const
    {
        s << (is_thread_fence_ ? "" : "compiler ")
            << format(mo_) << " fence";
    }
};




RL_INLINE
void atomic_thread_fence(memory_order mo, debug_info_param info)
{
    context& c = ctx();
    RL_VERIFY(false == c.invariant_executing);

    switch (mo)
    {
    case memory_order_relaxed:
        RL_VERIFY(false);
        break;
    case memory_order_consume:
    case memory_order_acquire:
        c.atomic_thread_fence_acquire();
        break;
    case memory_order_release:
        c.atomic_thread_fence_release();
        break;
    case memory_order_acq_rel:
        c.atomic_thread_fence_acq_rel();
        break;
    case memory_order_seq_cst:
        c.atomic_thread_fence_seq_cst();
        break;
    }

    RL_HIST(atomic_fence_event) {mo, true} RL_HIST_END();
}




RL_INLINE
void atomic_signal_fence(memory_order mo, debug_info_param info)
{
    context& c = ctx();
    RL_HIST(atomic_fence_event) {mo, false} RL_HIST_END();
}


}
