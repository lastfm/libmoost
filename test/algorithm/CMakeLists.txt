PROJECT(libmoost-algorithm-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_algorithm_test
               inplace_set_intersection
               ketama_partitioner
               variable_length_encoding
               main
               )

TARGET_LINK_LIBRARIES(moost_algorithm_test ${Boost_LIBRARIES})
