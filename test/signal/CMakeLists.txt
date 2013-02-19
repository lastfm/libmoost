PROJECT(libmoost-signal-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_signal_test
               signal_handler
               ../../src/signal/signal_handler
               main
               )

TARGET_LINK_LIBRARIES(moost_signal_test ${Boost_LIBRARIES})
