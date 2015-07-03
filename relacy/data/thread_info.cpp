/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "thread_info.hpp"

#include "../context_base.hpp"
#include "../foreach.hpp"
#include "atomic_data.hpp"

namespace rl
{

thread_info::thread_info(thread_id_t thread_count, thread_id_t index)
    : thread_info_base(thread_count, index)
    , sync_object_(thread_count)
    , acquire_fence_order_(thread_count)
    , release_fence_order_(thread_count)
{
}

void thread_info::iteration_begin()
{
    sync_object_.iteration_begin();
    last_yield_ = 0;
    dynamic_thread_func_ = 0;
    dynamic_thread_param_ = 0;
    std::fill(acq_rel_order_.begin(), acq_rel_order_.end(), 0);
    acq_rel_order_[index_] = 1;
    temp_switch_from_ = -1;
    saved_disable_preemption_ = -1;
}

void thread_info::on_start()
{
    RL_VERIFY(temp_switch_from_ == -1);
    RL_VERIFY(saved_disable_preemption_ == -1);
    sync_object_.on_start();
}

void thread_info::on_finish()
{
    RL_VERIFY(temp_switch_from_ == -1);
    RL_VERIFY(saved_disable_preemption_ == -1);
    sync_object_.on_finish();
}

void thread_info::atomic_thread_fence_acquire()
{
    assign_max(
        &acq_rel_order_[0],
        &acquire_fence_order_[0],
        acq_rel_order_.size());
}

void thread_info::atomic_thread_fence_release()
{
    std::copy(
        acq_rel_order_.begin(),
        acq_rel_order_.end(),
        release_fence_order_.begin());
}

void thread_info::atomic_thread_fence_acq_rel()
{
    atomic_thread_fence_acquire();
    atomic_thread_fence_release();
}

void thread_info::atomic_thread_fence_seq_cst(timestamp_t* seq_cst_fence_order)
{
    atomic_thread_fence_acquire();

    assign_max(
        &acq_rel_order_[0],
        seq_cst_fence_order,
        acq_rel_order_.size());

    std::copy(
        acq_rel_order_.begin(),
        acq_rel_order_.end(),
        seq_cst_fence_order);

    atomic_thread_fence_release();
}

thread_info::~thread_info()
{
}

unsigned thread_info::atomic_load_relaxed(atomic_data* RL_RESTRICT data)
{
    return atomic_load<memory_order_relaxed, false>(data);
}

unsigned thread_info::atomic_load_acquire(atomic_data* RL_RESTRICT data)
{
    return atomic_load<memory_order_acquire, false>(data);
}

unsigned thread_info::atomic_load_seq_cst(atomic_data* RL_RESTRICT data)
{
    return atomic_load<memory_order_seq_cst, false>(data);
}

unsigned thread_info::atomic_load_relaxed_rmw(atomic_data* RL_RESTRICT data)
{
    return atomic_load<memory_order_relaxed, true>(data);
}

unsigned thread_info::atomic_load_acquire_rmw(atomic_data* RL_RESTRICT data)
{
    return atomic_load<memory_order_acquire, true>(data);
}

unsigned thread_info::atomic_load_seq_cst_rmw(atomic_data* RL_RESTRICT data)
{
    return atomic_load<memory_order_seq_cst, true>(data);
}

unsigned thread_info::atomic_store_relaxed(atomic_data* RL_RESTRICT data)
{
    return atomic_store<memory_order_relaxed, false>(data);
}

unsigned thread_info::atomic_store_release(atomic_data* RL_RESTRICT data)
{
    return atomic_store<memory_order_release, false>(data);
}

unsigned thread_info::atomic_store_seq_cst(atomic_data* RL_RESTRICT data)
{
    return atomic_store<memory_order_seq_cst, false>(data);
}

unsigned thread_info::atomic_rmw_relaxed(atomic_data* RL_RESTRICT data, bool& aba)
{
    return atomic_rmw<memory_order_relaxed>(data, aba);
}

unsigned thread_info::atomic_rmw_acquire(atomic_data* RL_RESTRICT data, bool& aba)
{
    return atomic_rmw<memory_order_acquire>(data, aba);
}

unsigned thread_info::atomic_rmw_release(atomic_data* RL_RESTRICT data, bool& aba)
{
    return atomic_rmw<memory_order_release>(data, aba);
}

unsigned thread_info::atomic_rmw_acq_rel(atomic_data* RL_RESTRICT data, bool& aba)
{
    return atomic_rmw<memory_order_acq_rel>(data, aba);
}

unsigned thread_info::atomic_rmw_seq_cst(atomic_data* RL_RESTRICT data, bool& aba)
{
    return atomic_rmw<memory_order_seq_cst>(data, aba);
}

template<memory_order mo, bool rmw>
unsigned thread_info::get_load_index(atomic_data_impl& var)
{
    typedef atomic_data_impl::history_record history_t;

    unsigned index = var.current_index_;
    context& c = ctx();

    if (false == val(rmw))
    {
        size_t const limit = c.is_random_sched() ? atomic_history_size  - 1: 1;
        for (size_t i = 0; i != limit; ++i, --index)
        {
            history_t const& rec = var.history_[index % atomic_history_size];
            if (false == rec.busy_)
                return (unsigned)-1; // access to unitialized var

            history_t const& prev = var.history_[(index - 1) % atomic_history_size];
            if (prev.busy_ && prev.last_seen_order_[index_] <= last_yield_)
                break;

            if (memory_order_seq_cst == val(mo) && rec.seq_cst_)
                break;

            timestamp_t acq_rel_order =
                acq_rel_order_[rec.thread_id_];

            if (acq_rel_order >= rec.acq_rel_timestamp_)
                break;

            bool stop = false;
            for (thread_id_t i = 0; i != acq_rel_order_.size(); ++i)
            {
                timestamp_t acq_rel_order2 = acq_rel_order_[i];
                if (acq_rel_order2 >= rec.last_seen_order_[i])
                {
                    stop = true;
                    break;
                }
            }
            if (stop)
                break;

            if (0 == c.rand(2, sched_type_atomic_load))
                break;
        }
    }

    if (false == var.history_[index % atomic_history_size].busy_)
        return (unsigned)-1;

    return index;
}

template<memory_order mo, bool rmw>
unsigned thread_info::atomic_load(atomic_data* RL_RESTRICT data)
{
    RL_VERIFY(memory_order_release != mo || rmw);
    RL_VERIFY(memory_order_acq_rel != mo || rmw);

    atomic_data_impl& var = *static_cast<atomic_data_impl*>(data);

    typedef atomic_data_impl::history_record history_t;

    unsigned index = get_load_index<mo, rmw>(var);
    if ((unsigned)-1 == index)
        return (unsigned)-1;

    index %= atomic_history_size;
    history_t& rec = var.history_[index];
    RL_VERIFY(rec.busy_);

    own_acq_rel_order_ += 1;
    rec.last_seen_order_[index_] = own_acq_rel_order_;

    bool const synch =
        (memory_order_acquire == mo
        || memory_order_acq_rel == mo
        || memory_order_seq_cst == mo);

    timestamp_t* acq_rel_order = (synch ? &acq_rel_order_[0] : &acquire_fence_order_[0]);

    assign_max(
        acq_rel_order,
        &rec.acq_rel_order_[0],
        acq_rel_order_.size());

    return index;
}

unsigned thread_info::thread_info::atomic_init(atomic_data* RL_RESTRICT data)
{
    atomic_data_impl& var = *static_cast<atomic_data_impl*>(data);

    typedef atomic_data_impl::history_record history_t;

    unsigned const idx = ++var.current_index_ % atomic_history_size;
    history_t& rec = var.history_[idx];

    rec.busy_ = true;
    rec.thread_id_ = index_;
    rec.seq_cst_ = false;
    rec.acq_rel_timestamp_ = 0;

    std::fill(
        rec.acq_rel_order_.begin(),
        rec.acq_rel_order_.end(),
        0);

    return idx;
}

template<memory_order mo, bool rmw>
unsigned thread_info::atomic_store(atomic_data* RL_RESTRICT data)
{
    RL_VERIFY(memory_order_consume != mo || rmw);
    RL_VERIFY(memory_order_acquire != mo || rmw);
    RL_VERIFY(memory_order_acq_rel != mo || rmw);

    atomic_data_impl& var = *static_cast<atomic_data_impl*>(data);

    typedef atomic_data_impl::history_record history_t;

    unsigned const idx = ++var.current_index_ % atomic_history_size;
    history_t& rec = var.history_[idx];

    rec.busy_ = true;
    rec.thread_id_ = index_;
    rec.seq_cst_ = (memory_order_seq_cst == mo);

    own_acq_rel_order_ += 1;
    rec.acq_rel_timestamp_ = own_acq_rel_order_;

    std::fill(
        rec.last_seen_order_.begin(),
        rec.last_seen_order_.end(),
        (timestamp_t)-1);

    rec.last_seen_order_[index_] = own_acq_rel_order_;

    unsigned const prev_idx = (var.current_index_ - 1) % atomic_history_size;
    history_t& prev = var.history_[prev_idx];

    bool const synch =
        (memory_order_release == mo
        || memory_order_acq_rel == mo
        || memory_order_seq_cst == mo);

    bool const preserve =
        prev.busy_ && (rmw || (index_ == prev.thread_id_));

    timestamp_t* acq_rel_order = (synch ? &acq_rel_order_[0] : &release_fence_order_[0]);

    if (preserve)
    {
        std::copy(
            prev.acq_rel_order_.begin(),
            prev.acq_rel_order_.end(),
            rec.acq_rel_order_.begin());
        assign_max(&rec.acq_rel_order_[0], acq_rel_order, acq_rel_order_.size());
    }
    else
    {
        std::copy(
            acq_rel_order_.begin(),
            acq_rel_order_.end(),
            rec.acq_rel_order_.begin());
    }

    return idx;
}

template<memory_order mo>
unsigned thread_info::atomic_rmw(atomic_data* RL_RESTRICT data, bool& aba)
{
    atomic_data_impl& var = *static_cast<atomic_data_impl*>(data);
    timestamp_t const last_seen = var.history_[var.current_index_ % atomic_history_size].last_seen_order_[index_];
    aba = (last_seen > own_acq_rel_order_);
    atomic_load<mo, true>(data);
    unsigned result = atomic_store<mo, true>(data);

    return result;
}

unpark_reason thread_info::atomic_wait(atomic_data* RL_RESTRICT data, bool is_timed, bool allow_spurious_wakeup, debug_info_param info)
{
    context& c = ctx();
    atomic_data_impl& var = *static_cast<atomic_data_impl*>(data);
    unpark_reason const res = var.futex_ws_.park_current(c, is_timed, allow_spurious_wakeup, false, info);
    if (res == unpark_reason_normal)
        var.futex_sync_.acquire(this);
    return res;
}

thread_id_t thread_info::atomic_wake(atomic_data* RL_RESTRICT data, thread_id_t count, debug_info_param info)
{
    context& c = ctx();
    atomic_data_impl& var = *static_cast<atomic_data_impl*>(data);
    thread_id_t unblocked = 0;
    for (; count != 0; count -= 1, unblocked += 1)
    {
        if (var.futex_ws_.unpark_one(c, info) == false)
            break;
    }
    if (unblocked != 0)
        var.futex_sync_.release(this);
    return unblocked;
}

}
