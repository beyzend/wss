cmake_minimum_required(VERSION 2.8)

project(WssServer)

#include_directories("/home/beyzend/projects/tbb43_20141204oss/include") 
#include_directories("/usr/include/i386-linux-gnu/c++/4.8/")

#set( CMAKE_CXX_FLAGS "-std=c++11" )

if (CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
  # differentiation between debug and release builds.
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()
 
set(CMAKE_DEBUG_POSTFIX "_d")
 
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dist")


#file(GLOB_RECURSE HDRS "include/*.h")

add_subdirectory(src)
add_subdirectory(test)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/bin)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/media)

enable_testing()
add_test(NAME TestPath COMMAND TestPath)
add_test(NAME TestAttributes COMMAND TestAttributes) 
add_test(NAME TestScoreUtils COMMAND TestScoreUtils)

 
