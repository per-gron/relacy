#pragma once

#include "../relacy/relacy_std.hpp"



struct test_addr_hash : rl::test_suite<test_addr_hash, 2>
{
    void* p1;
    void* p2;
    size_t h1, h2;
    static size_t const table_size = 1000;

    void before()
    {
        p1 = rl::rl_malloc(0, $);
        h1 = rl::hash_ptr(p1, table_size);
        p2 = rl::rl_malloc(0, $);
        h2 = rl::hash_ptr(p2, table_size);
    }

    void after()
    {
        rl::rl_free(p1, $);
        rl::rl_free(p2, $);
    }

    void thread(unsigned index)
    {
        RL_ASSERT(h1 == rl::hash_ptr(p1, table_size));
        RL_ASSERT(h2 == rl::hash_ptr(p2, table_size));
        RL_ASSERT(rl::hash_ptr(&index, table_size) == rl::hash_ptr(&index,table_size));
        RL_ASSERT(rl::hash_ptr(0, table_size) == rl::hash_ptr(0, table_size));

    }
};


struct test_addr_hash2 : rl::test_suite<test_addr_hash2, 2, rl::test_result_until_condition_hit>
{
    static size_t const table_size = 4;
    rl::atomic<int> table [table_size];

    void before()
    {
        for (size_t i = 0; i != table_size; i += 1)
            table[i].store(0, rl::memory_order_relaxed, $);
    }

    void thread(unsigned)
    {
        for (size_t i = 0; i != table_size + 1; i += 1)
        {
            void* p = rl::rl_malloc(0, $);
            size_t idx = rl::hash_ptr(p, table_size);
            rl::rl_free(p, $);
            int v = table[idx].exchange(1, rl::memory_order_relaxed, $);
            RL_UNTIL(v);
        }
    }
};
