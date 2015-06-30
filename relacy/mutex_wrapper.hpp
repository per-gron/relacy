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

namespace rl
{

struct mutex_wrapper
{
    virtual void lock(debug_info_param info) const = 0;
    virtual void unlock(debug_info_param info) const = 0;
    virtual ~mutex_wrapper() {}
};

template<typename mutex_t>
class mutex_wrapper_impl : public mutex_wrapper
{
public:
    mutex_wrapper_impl(mutex_t& m)
        : m_(m)
    {
    }

    mutex_wrapper_impl(const mutex_wrapper_impl &) = delete;
    mutex_wrapper_impl &operator=(const mutex_wrapper_impl &) = delete;

private:
    mutex_t& m_;

    virtual void lock(debug_info_param info) const
    {
        m_.lock(info);
    }

    virtual void unlock(debug_info_param info) const
    {
        m_.unlock(info);
    }
};

}
