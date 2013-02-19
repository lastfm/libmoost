PROJECT(libmoost-mpl-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_mpl_test
               search
               main
               )

TARGET_LINK_LIBRARIES(moost_mpl_test ${Boost_LIBRARIES})
