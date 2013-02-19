PROJECT(libmoost-container-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_container_test
               bit_filter
               geo_map
               lru
               memory_mapped_dataset
               multi_map
               neigh_multi_map
               readers
               resource_stack
               simple_multi_map
               main
               )

TARGET_LINK_LIBRARIES(moost_container_test ${Boost_LIBRARIES})
