/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "generic_mutex_data.hpp"

#include "../context_base.hpp"

namespace rl
{

void generic_mutex_data::event_t::output(std::ostream& s) const
{
    s << "<" << std::hex << var_addr_ << std::dec << "> mutex: ";
    switch (type_)
    {
    case type_lock: s << "exclusive lock"; break;
    case type_unlock: s << "exclusive unlock"; break;
    case type_recursive_lock: s << "recursive exclusive lock"; break;
    case type_recursive_unlock: s << "recursive exclusive unlock"; break;
    case type_failed_try_lock: s << "failed exclusive try lock"; break;
    case type_spuriously_failed_try_lock: s << "spuriously failed exclusive try lock"; break;
    case type_lock_shared: s << "shared lock"; break;
    case type_unlock_shared: s << "shared unlock"; break;
    case type_recursive_lock_shared: s << "recursive shared lock"; break;
    case type_recursive_unlock_shared: s << "recursive shared unlock"; break;
    case type_failed_try_lock_shared: s << "failed shared try lock"; break;
    case type_spuriously_failed_try_lock_shared: s << "spuriously failed shared try lock"; break;
    case type_wait: s << "blocking"; break;
    case type_destroying_owned_mutex: s << "destroying owned mutex"; break;
    }
}

generic_mutex_data::generic_mutex_data(thread_id_t thread_count, bool is_rw, bool is_exclusive_recursive, bool is_shared_recursive, bool failing_try_lock)
    : is_rw_(is_rw)
    , is_exclusive_recursive_(is_exclusive_recursive)
    , is_shared_recursive_(is_shared_recursive)
    , failing_try_lock_(failing_try_lock)
    , sync_(thread_count)
    , exclusive_owner_(state_free)
    , exclusive_recursion_count_(0)
    , shared_owner_(thread_count)
    , exclusive_waitset_(thread_count)
    , shared_waitset_(thread_count)
    , shared_lock_count_(0)
    , try_lock_failed_()
{
    context& c = ctx();
    (void)c;
    RL_VERIFY(false == c.invariant_executing);
}

generic_mutex_data::~generic_mutex_data()
{
    context& c = ctx();
    RL_VERIFY(false == c.invariant_executing);
    if (exclusive_owner_ != state_free
        || exclusive_waitset_
        || shared_waitset_)
    {
        debug_info info = $;
        RL_HIST(event_t) {this, event_t::type_destroying_owned_mutex} RL_HIST_END();
        RL_ASSERT_IMPL(false, test_result_destroying_owned_mutex, "", $);
    }
}

bool generic_mutex_data::lock_exclusive(bool is_timed, debug_info_param info)
{
    context& c = ctx();
    c.sched();
    sign_.check(info);
    RL_VERIFY(false == c.invariant_executing);

    thread_id_t const my_id = c.threadx_->index_;

    if (exclusive_owner_ == state_shared && shared_owner_[my_id])
    {
        RL_HIST(event_t) {this, event_t::type_lock} RL_HIST_END();
        RL_ASSERT_IMPL(false, test_result_mutex_read_to_write_upgrade, "", info);
    }

    if (exclusive_owner_ == my_id)
    {
        RL_HIST(event_t) {this, event_t::type_recursive_lock} RL_HIST_END();
        if (is_exclusive_recursive_)
        {
            exclusive_recursion_count_ += 1;
            return true;
        }
        else
        {
            RL_ASSERT_IMPL(false, test_result_recursion_on_nonrecursive_mutex, "", info);
        }
    }

    for (;;)
    {
        if (exclusive_owner_ == state_free)
        {
            RL_VERIFY(exclusive_recursion_count_ == 0);
            //!!! in some implementation here must be acq_rel
            sync_.acquire(c.threadx_);
            exclusive_recursion_count_ = 1;
            exclusive_owner_ = my_id;
            RL_HIST(event_t) {this, event_t::type_lock} RL_HIST_END();
            return true;
        }
        else
        {
            RL_VERIFY(my_id != exclusive_owner_);
            RL_HIST(event_t) {this, event_t::type_wait} RL_HIST_END();
            unpark_reason reason = exclusive_waitset_.park_current(c, is_timed, false, false, info);
            RL_VERIFY(reason != unpark_reason_spurious);
            if (reason == unpark_reason_timeout)
            {
                sync_.acquire(c.threadx_);
                return false;
            }
        }

        //??? c.sched();
        //sign_.check(info);
    }
}

bool generic_mutex_data::try_lock_exclusive(debug_info_param info)
{
    context& c = ctx();
    c.sched();
    sign_.check(info);
    RL_VERIFY(false == c.invariant_executing);

    thread_id_t const my_id = c.threadx_->index_;

    if (exclusive_owner_ == state_shared && shared_owner_[my_id])
    {
        RL_HIST(event_t) {this, event_t::type_lock} RL_HIST_END();
        RL_ASSERT_IMPL(false, test_result_mutex_read_to_write_upgrade, "", info);
    }

    if (exclusive_owner_ == my_id)
    {
        RL_HIST(event_t) {this, event_t::type_recursive_lock} RL_HIST_END();
        if (is_exclusive_recursive_)
        {
            exclusive_recursion_count_ += 1;
            return true;
        }
        else
        {
            RL_ASSERT_IMPL(false, test_result_recursion_on_nonrecursive_mutex, "", info);
        }
    }

    if (exclusive_owner_ == state_free)
    {
        RL_VERIFY(exclusive_recursion_count_ == 0);
        //!!! probability rand
        if (true == failing_try_lock_
            && false == try_lock_failed_
            && c.rand(2, sched_type_user))
        {
            try_lock_failed_ = true;
            RL_HIST(event_t) {this, event_t::type_spuriously_failed_try_lock} RL_HIST_END();
            return false;
        }
        else
        {
            sync_.acquire(c.threadx_);
            exclusive_recursion_count_ = 1;
            exclusive_owner_ = my_id;
            RL_HIST(event_t) {this, event_t::type_lock} RL_HIST_END();
            return true;
        }
    }
    else
    {
        //!!! in some implementation here must be acquire
        //sync_.acquire(c.threadx_);

        RL_VERIFY(my_id != exclusive_owner_);
        RL_HIST(event_t) {this, event_t::type_failed_try_lock} RL_HIST_END();
        return false;
    }
}

void generic_mutex_data::unlock_exclusive(debug_info_param info)
{
    context& c = ctx();
    c.sched();
    sign_.check(info);
    RL_VERIFY(false == c.invariant_executing);

    thread_id_t const my_id = c.threadx_->index_;

    if (exclusive_owner_ != my_id)
    {
        RL_HIST(event_t) {this, event_t::type_unlock} RL_HIST_END();
        RL_ASSERT_IMPL(false, test_result_unlocking_mutex_wo_ownership, "", info);
    }

    exclusive_recursion_count_ -= 1;
    if (exclusive_recursion_count_)
    {
        RL_VERIFY(is_exclusive_recursive_);
        RL_HIST(event_t) {this, event_t::type_recursive_unlock} RL_HIST_END();
        return;
    }

    sync_.release(c.threadx_);
    exclusive_owner_ = state_free;
    RL_VERIFY(exclusive_recursion_count_ == 0);

    if (false == exclusive_waitset_.unpark_one(c, info))
        shared_waitset_.unpark_all(c, info);

    RL_HIST(event_t) {this, event_t::type_unlock} RL_HIST_END();
}

void generic_mutex_data::lock_shared(debug_info_param info)
{
    RL_VERIFY(is_rw_);
    context& c = ctx();
    c.sched();
    sign_.check(info);
    RL_VERIFY(false == c.invariant_executing);

    thread_id_t const my_id = c.threadx_->index_;

    if (exclusive_owner_ == my_id)
    {
        RL_HIST(event_t) {this, event_t::type_lock_shared} RL_HIST_END();
        RL_ASSERT_IMPL(false, test_result_mutex_write_to_read_upgrade, "", info);
    }

    if (exclusive_owner_ == state_shared && shared_owner_[my_id])
    {
        RL_HIST(event_t) {this, event_t::type_recursive_lock_shared} RL_HIST_END();
        if (is_shared_recursive_)
        {
            shared_owner_[my_id] += 1;
            shared_lock_count_ += 1;
            return;
        }
        else
        {
            RL_ASSERT_IMPL(false, test_result_recursion_on_nonrecursive_mutex, "", info);
        }
    }

    for (;;)
    {
        if ((exclusive_owner_ == state_free)
            || (exclusive_owner_ == state_shared
                && false == exclusive_waitset_))
        {
            sync_.acquire(c.threadx_);
            shared_owner_[my_id] += 1;
            shared_lock_count_ += 1;
            exclusive_owner_ = state_shared;
            RL_HIST(event_t) {this, event_t::type_lock_shared} RL_HIST_END();
            break;
        }
        else
        {
            RL_VERIFY(my_id != exclusive_owner_);
            RL_HIST(event_t) {this, event_t::type_wait} RL_HIST_END();
            shared_waitset_.park_current(c, false, false, false, info);
        }

        //??? c.sched();
        //sign_.check(info);
    }
}

bool generic_mutex_data::try_lock_shared(debug_info_param info)
{
    RL_VERIFY(is_rw_);
    context& c = ctx();
    c.sched();
    sign_.check(info);
    RL_VERIFY(false == c.invariant_executing);

    thread_id_t const my_id = c.threadx_->index_;

    if (exclusive_owner_ == my_id)
    {
        RL_HIST(event_t) {this, event_t::type_lock_shared} RL_HIST_END();
        RL_ASSERT_IMPL(false, test_result_mutex_write_to_read_upgrade, "", info);
    }

    if (exclusive_owner_ == state_shared && shared_owner_[my_id])
    {
        RL_HIST(event_t) {this, event_t::type_recursive_lock_shared} RL_HIST_END();
        if (is_shared_recursive_)
        {
            shared_owner_[my_id] += 1;
            shared_lock_count_ += 1;
            return true;
        }
        else
        {
            RL_ASSERT_IMPL(false, test_result_recursion_on_nonrecursive_mutex, "", info);
        }
    }

    if ((exclusive_owner_ == state_free)
        || (exclusive_owner_ == state_shared
            && false == exclusive_waitset_))
    {
        //!!! probability rand
        if (true == failing_try_lock_
            && false == try_lock_failed_
            && c.rand(2, sched_type_user))
        {
            try_lock_failed_ = true;
            RL_HIST(event_t) {this, event_t::type_spuriously_failed_try_lock_shared} RL_HIST_END();
            return false;
        }
        else
        {
            sync_.acquire(c.threadx_);
            shared_owner_[my_id] += 1;
            shared_lock_count_ += 1;
            exclusive_owner_ = state_shared;
            RL_HIST(event_t) {this, event_t::type_lock_shared} RL_HIST_END();
            return true;
        }
    }
    else
    {
        RL_VERIFY(my_id != exclusive_owner_);
        RL_HIST(event_t) {this, event_t::type_failed_try_lock_shared} RL_HIST_END();
        return false;
    }
}

void generic_mutex_data::unlock_shared(debug_info_param info)
{
    RL_VERIFY(is_rw_);
    context& c = ctx();
    c.sched();
    sign_.check(info);
    RL_VERIFY(false == c.invariant_executing);

    thread_id_t const my_id = c.threadx_->index_;

    if (exclusive_owner_ != state_shared || 0 == shared_owner_[my_id])
    {
        RL_HIST(event_t) {this, event_t::type_unlock_shared} RL_HIST_END();
        RL_ASSERT_IMPL(false, test_result_unlocking_mutex_wo_ownership, "", info);
    }

    RL_VERIFY(shared_lock_count_);
    shared_owner_[my_id] -= 1;
    shared_lock_count_ -= 1;
    if (shared_lock_count_ != 0)
    {
        if (shared_owner_[my_id])
        {
            RL_VERIFY(is_shared_recursive_);
            RL_HIST(event_t) {this, event_t::type_recursive_unlock_shared} RL_HIST_END();
        }
        else
        {
            sync_.release(c.threadx_);
            RL_HIST(event_t) {this, event_t::type_unlock_shared} RL_HIST_END();
        }
        return;
    }

    sync_.release(c.threadx_);
    exclusive_owner_ = state_free;

    exclusive_waitset_.unpark_one(c, info);

    RL_HIST(event_t) {this, event_t::type_unlock_shared} RL_HIST_END();
}

void generic_mutex_data::unlock_exclusive_or_shared(debug_info_param info)
{
    if (exclusive_owner_ == ctx().threadx_->index_)
        unlock_exclusive(info);
    else
        unlock_shared(info);
}

bool generic_mutex_data::is_signaled(debug_info_param info)
{
    (void)info;
    return (exclusive_owner_ == state_free);
}

void generic_mutex_data::memory_acquire(debug_info_param info)
{
    (void)info;
    sync_.acquire(ctx().threadx_);
}

void* generic_mutex_data::prepare_wait(debug_info_param info)
{
    (void)info;
    return &exclusive_waitset_;
}

}
