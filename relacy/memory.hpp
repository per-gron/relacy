/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#pragma once

#include <iostream>
#include <map>

#include "base.hpp"

namespace rl
{


class memory_mgr : nocopy<>
{
public:
    memory_mgr();

    ~memory_mgr();

    void* alloc(size_t size);

    bool free(void* pp, bool defer);

    bool iteration_end();

    void output_allocs(std::ostream& stream);

private:
    typedef rl_stack<void*>                 freelist_t;
    typedef std::pair<size_t, freelist_t>   alloc_entry_t;
    typedef rl_vector<alloc_entry_t>        alloc_t;

    static size_t const deferred_count      = 64;

    alloc_t alloc_cache_;
    size_t deferred_index_;
    void* deferred_free_ [deferred_count];
    size_t deferred_free_size_ [deferred_count];

    rl_map<void*, size_t> allocs_;

    void free_impl(void* p, size_t size);
};




struct memory_alloc_event
{
    void*                       addr_;
    size_t                      size_;
    bool                        is_array_;

    void output(std::ostream& s) const;
};


struct memory_free_event
{
    void*                       addr_;
    bool                        is_array_;

    void output(std::ostream& s) const;
};

}
