/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "memory.hpp"

namespace rl
{

memory_mgr::memory_mgr()
{
    memset(deferred_free_, 0, sizeof(deferred_free_));
    memset(deferred_free_size_, 0, sizeof(deferred_free_size_));
    deferred_index_ = 0;
}

memory_mgr::~memory_mgr()
{
    /*
    while (allocs_.size())
    {
        size_t* p = (size_t*)(allocs_.begin()->first);
        free(p - 1, false);
        allocs_.erase(allocs_.begin());
    }
    */
}

void* memory_mgr::alloc(size_t size)
{
    void* pp = 0;
    for (size_t i = 0; i != alloc_cache_.size(); ++i)
    {
        if (alloc_cache_[i].first == size)
        {
            if (alloc_cache_[i].second.size())
            {
                pp = alloc_cache_[i].second.top();
                alloc_cache_[i].second.pop();
            }
            break;
        }
    }
    if (0 == pp)
        pp = (::malloc)(size + alignment);

    if (pp)
    {
        RL_VERIFY(alignment >= sizeof(void*));
        *(size_t*)pp = size;
        void* p = (char*)pp + alignment;
        allocs_.insert(std::make_pair(p, size));
        return p;
    }
    else
    {
        throw std::bad_alloc();
    }
}

bool memory_mgr::free(void* pp, bool defer)
{
    if (0 == pp)
        return true;

    rl_map<void*, size_t>::iterator iter = allocs_.find(pp);
    if (allocs_.end() == iter)
        return false;

    allocs_.erase(iter);

    void* p = (char*)pp - alignment;
    size_t size = *(size_t*)p;

    if (defer)
    {
        deferred_free_[deferred_index_ % deferred_count] = p;
        deferred_free_size_[deferred_index_ % deferred_count] = size;
        deferred_index_ += 1;
        p = deferred_free_[deferred_index_ % deferred_count];
        size = deferred_free_size_[deferred_index_ % deferred_count];
        if (p)
            free_impl(p, size);
    }
    else
    {
        free_impl(p, size);
    }
    return true;
}

bool memory_mgr::iteration_end()
{
    return allocs_.empty();
}

void memory_mgr::output_allocs(std::ostream& stream)
{
    stream << "memory allocations:" << std::endl;
    rl_map<void*, size_t>::iterator iter = allocs_.begin();
    rl_map<void*, size_t>::iterator end = allocs_.end();
    for (; iter != end; ++iter)
    {
        stream << iter->first << " [" << (unsigned)iter->second << "]" << std::endl;
    }
    stream << std::endl;
}

void memory_mgr::free_impl(void* p, size_t size)
{
    bool found = false;
    for (size_t i = 0; i != alloc_cache_.size(); ++i)
    {
        if (alloc_cache_[i].first == size)
        {
            found = true;
            alloc_cache_[i].second.push(p);
            break;
        }
    }
    if (!found)
    {
        alloc_cache_.push_back(std::make_pair(size, freelist_t()));
        alloc_cache_.back().second.push(p);
    }
}

void memory_alloc_event::output(std::ostream& s) const
{
    s << "memory allocation: addr=" << std::hex << (void*)((char*)addr_ + (is_array_ ? alignment : 0)) << std::dec
        << ", size=" << (unsigned)size_;
}

void memory_free_event::output(std::ostream& s) const
{
    s << "memory deallocation: addr=" << std::hex << (void*)((char*)addr_ + (is_array_ ? alignment : 0)) << std::dec;
}

}
