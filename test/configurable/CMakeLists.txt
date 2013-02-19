PROJECT(libmoost-configurable-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_configurable_test
               configurable
               ../../src/configurable/binder
               main
               )

TARGET_LINK_LIBRARIES(moost_configurable_test ${Boost_LIBRARIES})
