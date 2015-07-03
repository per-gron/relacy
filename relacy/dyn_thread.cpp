/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "dyn_thread.hpp"

#include "context_base.hpp"

namespace rl
{

dyn_thread::dyn_thread()
{
    handle_ = 0;
}

void dyn_thread::start(void*(*fn)(void*), void* arg)
{
    RL_VERIFY(handle_ == 0);
    handle_ = ctx().create_thread(fn, arg);
}

void dyn_thread::join()
{
    RL_VERIFY(handle_);
    handle_->wait(false, false, $);
    handle_ = 0;
}

}
