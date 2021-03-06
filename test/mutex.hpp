#pragma once

#include "../relacy/relacy.hpp"

struct test_mutex : rl::test_suite<test_mutex, 3>
{
    rl::mutex mtx;
    rl::var<int> data;

    void before()
    {
        data($) = 0;
    }

    void after()
    {
        RL_ASSERT(data($) == 3);
    }

    void thread(unsigned /*index*/)
    {
        mtx.lock($);
        data($) += 1;
        mtx.unlock($);
    }
};




struct test_deadlock : rl::test_suite<test_deadlock, 2, rl::test_result_deadlock>
{
    rl::mutex mtx1;
    rl::mutex mtx2;

    void thread(unsigned index)
    {
        if (0 == index)
        {
            mtx1.lock($);
            mtx2.lock($);
            mtx1.unlock($);
            mtx2.unlock($);
        }
        else
        {
            mtx2.lock($);
            mtx1.lock($);
            mtx1.unlock($);
            mtx2.unlock($);
        }
    }
};



struct test_deadlock2 : rl::test_suite<test_deadlock2, 2, rl::test_result_deadlock>
{
    rl::mutex m;
    rl::atomic<int> f;

    void before()
    {
        f($) = 0;
    }

    void thread(unsigned index)
    {
        if (index)
        {
            m.lock($);
            f($) = 1;
            for (int i = 0; i != 100; ++i)
                rl::yield(1, $);
        }
        else
        {
            while (0 == f($))
                rl::yield(1, $);
            m.lock($);
        }
    }
};



struct test_mutex_destuction : rl::test_suite<test_mutex_destuction, 1, rl::test_result_destroying_owned_mutex>
{
    void thread(unsigned)
    {
        rl::mutex* m = RL_NEW rl::mutex;
        m->lock($);
        RL_DELETE m;
    }
};


struct test_mutex_destuction2 : rl::test_suite<test_mutex_destuction2, 2, rl::test_result_destroying_owned_mutex>
{
    rl::mutex* m;
    rl::atomic<int> f;

    void before()
    {
        m = RL_NEW rl::mutex;
        f($) = 0;
    }

    void thread(unsigned index)
    {
        if (0 == index)
        {
            m->lock($);
            f($) = 1;
            while (1 == f($))
                rl::yield(1, $);
            m->unlock($);
        }
        else
        {
            while (0 == f($))
                rl::yield(1, $);
            RL_DELETE m;
            f($) = 2;
        }
    }
};


struct test_mutex_recursion : rl::test_suite<test_mutex_recursion, 2>
{
    rl::recursive_mutex mtx;
    rl::var<int> data;

    void before()
    {
        data($) = 0;
    }

    void after()
    {
        RL_ASSERT(data($) == 2);
    }

    void thread(unsigned /*index*/)
    {
        mtx.lock($);
        mtx.lock($);
        data($) += 1;
        mtx.unlock($);
        mtx.unlock($);
    }
};



struct test_mutex_try_lock : rl::test_suite<test_mutex_try_lock, 2>
{
    rl::recursive_mutex mtx;
    rl::var<int> data;

    void before()
    {
        data($) = 0;
    }

    void after()
    {
        RL_ASSERT(data($) == 2);
    }

    void thread(unsigned /*index*/)
    {
        while (false == mtx.try_lock($))
            rl::yield(1, $);
        RL_ASSERT(mtx.try_lock($));
        data($) += 1;
        mtx.unlock($);
        mtx.unlock($);
    }
};



struct test_mutex_recursion_error : rl::test_suite<test_mutex_recursion_error, 1, rl::test_result_recursion_on_nonrecursive_mutex>
{
    void thread(unsigned)
    {
        rl::mutex m;
        m.lock($);
        m.lock($);
    }
};



struct test_mutex_unlock_error : rl::test_suite<test_mutex_unlock_error, 1, rl::test_result_unlocking_mutex_wo_ownership>
{
    void thread(unsigned)
    {
        rl::mutex m;
        m.lock($);
        m.unlock($);
        m.unlock($);
    }
};


struct test_mutex_leak : rl::test_suite<test_mutex_leak, 1, rl::test_result_resource_leak>
{
    void thread(unsigned)
    {
        char* p = RL_NEW char [sizeof(rl::mutex)];
        RL_NEW (p) rl::mutex();
        RL_DELETE [] p;
    }
};


