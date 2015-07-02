/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "sync_var.hpp"

#include "data/thread_info_base.hpp"
#include "foreach.hpp"

namespace rl
{

sync_var::sync_var(thread_id_t thread_count)
    : order_(thread_count)
{
    iteration_begin();
}

void sync_var::iteration_begin()
{
    std::fill(order_.begin(), order_.end(), 0);
}

void sync_var::acquire(thread_info_base* th)
{
    th->own_acq_rel_order_ += 1;
    assign_max(&th->acq_rel_order_[0], &order_[0], order_.size());
}

void sync_var::release(thread_info_base* th)
{
    th->own_acq_rel_order_ += 1;
    assign_max(&order_[0], &th->acq_rel_order_[0], order_.size());
}

void sync_var::acq_rel(thread_info_base* th)
{
    th->own_acq_rel_order_ += 1;
    timestamp_t* acq_rel_order = &th->acq_rel_order_[0];
    timestamp_t* order = &order_[0];
    assign_max(acq_rel_order, order, order_.size());
    assign_max(order, acq_rel_order, order_.size());
}

}
