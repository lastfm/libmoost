PROJECT(libmoost-string-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_string_test
               levenshtein
               main
               )

TARGET_LINK_LIBRARIES(moost_string_test ${Boost_LIBRARIES})
