/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "memory_order.hpp"

namespace rl
{

char const* format(memory_order mo)
{
    switch (mo)
    {
    case memory_order_relaxed: return "relaxed";
    case memory_order_consume: return "consume";
    case memory_order_acquire: return "acquire";
    case memory_order_release: return "release";
    case memory_order_acq_rel: return "acq_rel";
    case memory_order_seq_cst: return "seq_cst";
    }
    RL_VERIFY(!"invalid value of memory order");
    throw std::logic_error("invalid value of memory order");
}

}
