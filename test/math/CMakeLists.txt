PROJECT(libmoost-math-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_math_test
               log2
               main
               )

TARGET_LINK_LIBRARIES(moost_math_test ${Boost_LIBRARIES})
