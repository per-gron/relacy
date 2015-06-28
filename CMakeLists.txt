# Use the (not yet ready) standalone build system for CompilerRT. In order to
# trick it into using the standalone build, CMAKE_SOURCE_DIR is manipulated.
set(ACTUAL_CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/vendor/compiler-rt")
add_subdirectory(vendor/compiler-rt EXCLUDE_FROM_ALL)
set(CMAKE_SOURCE_DIR ${ACTUAL_CMAKE_SOURCE_DIR})
