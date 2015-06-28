#pragma once

#include "../relacy/relacy_std.hpp"


struct test_memory_allocation : rl::test_suite<test_memory_allocation, 2>
{
    void thread(unsigned /*index*/)
    {
        VAR_T(int)* p1 = RL_NEW_PROXY VAR_T(int) (5), i1 = 5, * p11 = RL_NEW_PROXY VAR_T(int) (6);
        VAR(p1[0]) = 1;
        RL_DELETE_PROXY p1, RL_DELETE_PROXY p11;

        VAR_T(int)* p2 = RL_NEW_PROXY VAR_T(int) [10], i2 = 6, *p22 = RL_NEW_PROXY VAR_T(int) [20];
        VAR(p2[0]) = 1;
        RL_DELETE_PROXY [] p2, RL_DELETE_PROXY [] p22;

        void* p3 = rl::malloc(10, $), *i3 = 0, *p33 = rl::malloc(20, $);
        rl::free(p3, $), rl::free(p33, $);

        void* p4 = rl::malloc(sizeof(int), $);
        int* i4 = RL_NEW_PROXY (p4) int (11);
        rl::free(p4, $);

        //RL_ASSERT(false);
        (void)i1, (void)i2, (void)i3; (void)i4;
    }
};

