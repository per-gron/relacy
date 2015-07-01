/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#import "context_base.hpp"

namespace rl
{

void context::disable_preemption()
{
    disable_preemption_ += 1;
}

void context::enable_preemption()
{
    disable_preemption_ -= 1;
}

int context::get_errno()
{
    RL_VERIFY(threadx_);
    return threadx_->errno_;
}

void context::set_errno(int value)
{
    RL_VERIFY(threadx_);
    threadx_->errno_ = value;
}

}
