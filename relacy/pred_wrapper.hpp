/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#pragma once

namespace rl
{

struct pred_wrapper
{
    virtual bool exec() const = 0;
    virtual ~pred_wrapper() {}
};

template<typename pred_t>
class pred_wrapper_impl : public pred_wrapper
{
public:
    pred_wrapper_impl(pred_t p)
        : p_(p)
    {
    }

    pred_wrapper_impl(const pred_wrapper_impl &) = delete;
    pred_wrapper_impl &operator=(const pred_wrapper_impl &) = delete;

private:
    mutable pred_t p_;

    virtual bool exec() const
    {
        return p_();
    }
};

}
