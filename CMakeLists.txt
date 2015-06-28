cmake_minimum_required(VERSION 2.8)
project(relacy)

# Use the (not yet ready) standalone build system for CompilerRT. In order to
# trick it into using the standalone build, CMAKE_SOURCE_DIR is manipulated.
set(ACTUAL_CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/vendor/compiler-rt")
add_subdirectory(vendor/compiler-rt EXCLUDE_FROM_ALL)
set(CMAKE_SOURCE_DIR ${ACTUAL_CMAKE_SOURCE_DIR})

add_definitions(
  -std=c++11
  -stdlib=libc++
)

set(relacy_sources
  relacy/atomic.hpp
  relacy/atomic_events.hpp
  relacy/atomic_fence.hpp
  relacy/backoff.hpp
  relacy/base.hpp
  relacy/context.hpp
  relacy/context_addr_hash.hpp
  relacy/context_base.hpp
  relacy/context_base_impl.hpp
  relacy/context_bound_scheduler.hpp
  relacy/defs.hpp
  relacy/dyn_thread.hpp
  relacy/dyn_thread_ctx.hpp
  relacy/foreach.hpp
  relacy/full_search_scheduler.hpp
  relacy/history.hpp
  relacy/memory.hpp
  relacy/memory_order.hpp
  relacy/pch.hpp
  relacy/platform.hpp
  relacy/random.hpp
  relacy/random_scheduler.hpp
  relacy/relacy.hpp
  relacy/relacy_std.hpp
  relacy/rmw.hpp
  relacy/scheduler.hpp
  relacy/signature.hpp
  relacy/slab_allocator.hpp
  relacy/stdlib
  relacy/stdlib/condition_variable.hpp
  relacy/stdlib/event.hpp
  relacy/stdlib/mutex.hpp
  relacy/stdlib/pthread.hpp
  relacy/stdlib/semaphore.hpp
  relacy/stdlib/windows.hpp
  relacy/sync_var.hpp
  relacy/test_params.hpp
  relacy/test_result.hpp
  relacy/test_suite.hpp
  relacy/thread.hpp
  relacy/thread_base.hpp
  relacy/thread_local.hpp
  relacy/thread_local_ctx.hpp
  relacy/var.hpp
  relacy/waitset.hpp)

set(relacy_test_sources
  test/addr_hash.hpp
  test/compare_swap.hpp
  test/condvar.hpp
  test/data_race.hpp
  test/dyn_thread.hpp
  test/event.hpp
  test/fence.hpp
  test/futex.hpp
  test/main.cpp
  test/memory.hpp
  test/memory_order.hpp
  test/mutex.hpp
  test/pthread.hpp
  test/scheduler.hpp
  test/semaphore.hpp
  test/thread_local.hpp
  test/wfmo.hpp
  test/windows.hpp)

add_executable(relacy_test ${relacy_test_sources})
