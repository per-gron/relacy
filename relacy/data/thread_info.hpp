/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#pragma once

#include "../base.hpp"
#include "../context_base.hpp"
#include "../memory_order.hpp"
#include "../test_suite.hpp"
#include "../thread_sync_object.hpp"
#include "atomic_data.hpp"
#include "thread_info_base.hpp"

namespace rl
{

struct atomic_data;
struct var_data;
struct atomic_data_impl;
struct var_data_impl;

struct thread_info : thread_info_base
{
    thread_info(thread_id_t thread_count = 0, thread_id_t index = 0);

    thread_info(thread_info const&) = delete;
    thread_info& operator = (thread_info const&) = delete;

    void iteration_begin();

    thread_sync_object sync_object_;
    rl_vector<timestamp_t> acquire_fence_order_;
    rl_vector<timestamp_t> release_fence_order_;

    virtual void on_start();

    virtual void on_finish();

    void atomic_thread_fence_acquire();

    void atomic_thread_fence_release();

    void atomic_thread_fence_acq_rel();

    void atomic_thread_fence_seq_cst(timestamp_t* seq_cst_fence_order);

    virtual ~thread_info(); // just to calm down gcc

private:
    virtual unsigned atomic_load_relaxed(atomic_data* RL_RESTRICT data);

    virtual unsigned atomic_load_acquire(atomic_data* RL_RESTRICT data);

    virtual unsigned atomic_load_seq_cst(atomic_data* RL_RESTRICT data);

    virtual unsigned atomic_load_relaxed_rmw(atomic_data* RL_RESTRICT data);

    virtual unsigned atomic_load_acquire_rmw(atomic_data* RL_RESTRICT data);

    virtual unsigned atomic_load_seq_cst_rmw(atomic_data* RL_RESTRICT data);

    virtual unsigned atomic_store_relaxed(atomic_data* RL_RESTRICT data);

    virtual unsigned atomic_store_release(atomic_data* RL_RESTRICT data);

    virtual unsigned atomic_store_seq_cst(atomic_data* RL_RESTRICT data);

    virtual unsigned atomic_rmw_relaxed(atomic_data* RL_RESTRICT data, bool& aba);

    virtual unsigned atomic_rmw_acquire(atomic_data* RL_RESTRICT data, bool& aba);

    virtual unsigned atomic_rmw_release(atomic_data* RL_RESTRICT data, bool& aba);

    virtual unsigned atomic_rmw_acq_rel(atomic_data* RL_RESTRICT data, bool& aba);

    virtual unsigned atomic_rmw_seq_cst(atomic_data* RL_RESTRICT data, bool& aba);

    template<memory_order mo, bool rmw>
    unsigned get_load_index(atomic_data_impl& var);

    template<memory_order mo, bool rmw>
    unsigned atomic_load(atomic_data* RL_RESTRICT data);

    virtual unsigned atomic_init(atomic_data* RL_RESTRICT data);

    template<memory_order mo, bool rmw>
    unsigned atomic_store(atomic_data* RL_RESTRICT data);

    template<memory_order mo>
    unsigned atomic_rmw(atomic_data* RL_RESTRICT data, bool& aba);

    virtual unpark_reason atomic_wait(atomic_data* RL_RESTRICT data, bool is_timed, bool allow_spurious_wakeup, debug_info_param info);

    virtual thread_id_t atomic_wake(atomic_data* RL_RESTRICT data, thread_id_t count, debug_info_param info);
};

}
