PROJECT(libmoost-io-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_io_test
               async_writer
               block_store
               file_backed_data_source
               file_operations
               file_watcher
               ionotify
               map_store
               variable_store
               main
               )

TARGET_LINK_LIBRARIES(moost_io_test ${Log4cxx_LIBRARIES} ${Boost_LIBRARIES})
