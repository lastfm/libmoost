PROJECT(libmoost-digest-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_digest_test
               sha2
               main
               )

TARGET_LINK_LIBRARIES(moost_digest_test
                      moost_core
                      ${Boost_LIBRARIES})
