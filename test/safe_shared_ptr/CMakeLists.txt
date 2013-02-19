PROJECT(libmoost-safe_shared_ptr-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_safe_shared_ptr_test
               safe_shared_ptr
               main
               )

TARGET_LINK_LIBRARIES(moost_safe_shared_ptr_test ${Boost_LIBRARIES})
