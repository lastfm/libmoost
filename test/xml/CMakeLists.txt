PROJECT(libmoost-xml-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_xml_test
               simple_parser
               main
               )

TARGET_LINK_LIBRARIES(moost_xml_test ${Boost_LIBRARIES})
