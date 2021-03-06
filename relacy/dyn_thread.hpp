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
#include "stdlib/waitable_object.hpp"


namespace rl
{

class dyn_thread
{
public:
    dyn_thread();

    dyn_thread(const dyn_thread &) = delete;
    dyn_thread &operator=(const dyn_thread &) = delete;

    void start(void*(*fn)(void*), void* arg);

    void join();

private:
    win_waitable_object* handle_;
};

}
