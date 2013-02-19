PROJECT(libmoost-posixtime-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_posix_time_test
               timestamp
               main
               )

TARGET_LINK_LIBRARIES(moost_posix_time_test ${Boost_LIBRARIES})
