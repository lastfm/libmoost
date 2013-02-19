PROJECT(libmoost-kvstore-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_kvstore_test
               kvstore_client_test
               main
               )

TARGET_LINK_LIBRARIES(moost_kvstore_test ${Boost_LIBRARIES})
