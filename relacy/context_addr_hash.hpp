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
#include "defs.hpp"
#include "test_params.hpp"


namespace rl
{

class context_addr_hash
{
public:
    void iteration_begin()
    {
        hash_map_.clear();
        hash_seq_ = 0;
    }

    size_t get_addr_hash(void const* p)
    {
        //!!! accept 'table size' to do 'hash % table_size'
        // will give more information for state exploration

        hash_map_t::iterator iter (hash_map_.find(p));
        if (iter != hash_map_.end() && iter->first == p)
        {
            return iter->second;
        }
        else
        {
            //!!! distribute hashes more randomly, use rand()
            size_t hash = hash_seq_++;
            hash_map_.insert(std::make_pair(p, hash));
            return hash;
        }
    }

private:
    struct entry
    {
        uintptr_t       ptr_;
        size_t          hash_;
    };
    typedef rl_map<void const*, size_t> hash_map_t;
    hash_map_t                          hash_map_;
    size_t                              hash_seq_;
};


}
