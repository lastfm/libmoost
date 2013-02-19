PROJECT(libmoost-kvds-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_kvds_test
               ikvds
               kvds_key_iterator
               kvds
               main
               )

TARGET_LINK_LIBRARIES(moost_kvds_test tokyocabinet kyotocabinet db_cxx ${Boost_LIBRARIES})
