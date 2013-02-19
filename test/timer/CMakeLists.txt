PROJECT(libmoost-timer-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_timer_test
               main
               timer
               ../../src/timer
               )

TARGET_LINK_LIBRARIES(moost_timer_test ${Boost_LIBRARIES})
