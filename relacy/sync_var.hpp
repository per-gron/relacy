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
#include "foreach.hpp"

namespace rl
{


class sync_var
{
public:
    sync_var(thread_id_t thread_count)
        : order_(thread_count)
    {
        iteration_begin();
    }

    sync_var(const sync_var &) = delete;
    sync_var &operator=(const sync_var &) = delete;

    void iteration_begin()
    {
        std::fill(order_.begin(), order_.end(), 0);
    }

    void acquire(thread_info_base* th)
    {
        th->own_acq_rel_order_ += 1;
        assign_max(th->acq_rel_order_, &order_[0], order_.size());
    }

    void release(thread_info_base* th)
    {
        th->own_acq_rel_order_ += 1;
        assign_max(&order_[0], th->acq_rel_order_, order_.size());
    }

    void acq_rel(thread_info_base* th)
    {
        th->own_acq_rel_order_ += 1;
        timestamp_t* acq_rel_order = th->acq_rel_order_;
        timestamp_t* order = &order_[0];
        assign_max(acq_rel_order, order, order_.size());
        assign_max(order, acq_rel_order, order_.size());
    }

private:
    template<typename T>
    void assign_max(T *target, T *compare, size_t count) {
        for (size_t i = 0; i < count; i++) {
            if (compare[i] > target[i]) {
                target[i] = compare[i];
            }
        }
    }

    rl_vector<timestamp_t> order_;
};

}
