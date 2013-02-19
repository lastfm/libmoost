PROJECT(libmoost-utils-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_utils_test
               fixed_interval_timer
               scope_exit
               bits
               stringify
               relops
               main
               )

TARGET_LINK_LIBRARIES(moost_utils_test ${Boost_LIBRARIES})
