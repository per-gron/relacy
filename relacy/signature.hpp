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
#include "test_result.hpp"


namespace rl
{

namespace detail
{

RL_NOINLINE void fail(const void *address, debug_info_param info);

}

template<unsigned magic>
class signature
{
public:
    signature()
        : magic_(magic)
    {
    }

    signature(signature const&)
        : magic_(magic)
    {
    }

    ~signature()
    {
        check(RL_INFO);
        magic_ = 0;
    }

    void check(debug_info_param info) const
    {
        if (
            ((uintptr_t)this <= (uintptr_t)-1 - 4096) &&
            ((uintptr_t)this >= 4096) &&
            ((uintptr_t)this % sizeof(unsigned) == 0) && (magic == magic_))
        {
            return;
        }
        else
        {
            detail::fail(this, info);
        }
    }

private:
    unsigned magic_;
};

}
