PROJECT(libmoost-hash-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_hash_test
               murmur3
               main
               )

TARGET_LINK_LIBRARIES(moost_hash_test ${Boost_LIBRARIES})
