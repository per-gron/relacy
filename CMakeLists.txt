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
  relacy/base.hpp
  relacy/context.hpp
  relacy/context_addr_hash.hpp
  relacy/context_base.cpp
  relacy/context_base.hpp
  relacy/context_bound_scheduler.hpp
  relacy/data/atomic_data.cpp
  relacy/data/atomic_data.hpp
  relacy/data/condvar_data.cpp
  relacy/data/condvar_data.hpp
  relacy/data/event_data.cpp
  relacy/data/event_data.hpp
  relacy/data/generic_mutex_data.cpp
  relacy/data/generic_mutex_data.hpp
  relacy/data/sema_data.cpp
  relacy/data/sema_data.hpp
  relacy/data/thread_info.cpp
  relacy/data/thread_info.hpp
  relacy/data/var_data.cpp
  relacy/data/var_data.hpp
  relacy/defs.hpp
  relacy/dyn_thread.hpp
  relacy/foreach.hpp
  relacy/full_search_scheduler.hpp
  relacy/history.cpp
  relacy/history.hpp
  relacy/memory.cpp
  relacy/memory.hpp
  relacy/memory_order.cpp
  relacy/memory_order.hpp
  relacy/mutex_wrapper.hpp
  relacy/platform.hpp
  relacy/random.hpp
  relacy/random_scheduler.hpp
  relacy/relacy.hpp
  relacy/rmw.cpp
  relacy/rmw.hpp
  relacy/scheduler.hpp
  relacy/signature.cpp
  relacy/signature.hpp
  relacy/slab_allocator.hpp
  relacy/stdlib
  relacy/stdlib/condition_variable.hpp
  relacy/stdlib/event.hpp
  relacy/stdlib/mutex.hpp
  relacy/stdlib/pthread.cpp
  relacy/stdlib/pthread.hpp
  relacy/stdlib/semaphore.cpp
  relacy/stdlib/semaphore.hpp
  relacy/stdlib/waitable_object.hpp
  relacy/stdlib/windows.cpp
  relacy/stdlib/windows.hpp
  relacy/sync_var.cpp
  relacy/sync_var.hpp
  relacy/test_params.cpp
  relacy/test_params.hpp
  relacy/test_result.hpp
  relacy/test_suite.hpp
  relacy/thread_local.hpp
  relacy/thread_local_ctx.hpp
  relacy/thread_sync_object.cpp
  relacy/thread_sync_object.hpp
  relacy/var.hpp
  relacy/waitset.cpp
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

add_executable(relacy_test ${relacy_sources} ${relacy_test_sources})
