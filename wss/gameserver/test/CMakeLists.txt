find_library(LIB_GTEST gtest PATHS ${test_SOURCE_DIR})
find_library(LIB_GTEST_MAIN gtest_main PATHS ${test_SOURCE_DIR})

include_directories(${test_SOURCE_DIR}/src/include}
  ${WssServer_SOURCE_DIR}/include)


#set( CMAKE_CXX_FLAGS "-std=c++11" )

include_directories("${CMAKE_SOURCE_DIR}/test/include")
include_directories("${CMAKE_SOURCE_DIR}/include/")


add_executable(TestPath TestPath.cpp)
target_link_libraries(TestPath
  WssServerLib
  ${LIB_GTEST}
  ${LIB_GTEST_MAIN}
  ${LIB_PTHREAD}
  )
 

