PROJECT(libmoost-pdl-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_pdl_test
               test_pdl
               test_multithread
               main
               )

# TODO: dl is system dependent!
TARGET_LINK_LIBRARIES(moost_pdl_test moost_pdl dl ${Boost_LIBRARIES})

ADD_SHARED_LIBRARY( test_module1 test_class_1.cpp )
SET_TARGET_PROPERTIES( test_module1 PROPERTIES OUTPUT_NAME test_module1 )

ADD_SHARED_LIBRARY( test_module2 test_class_2.cpp )
SET_TARGET_PROPERTIES( test_module2 PROPERTIES OUTPUT_NAME test_module2 )

ADD_SHARED_LIBRARY( test_module3 test_class_3.cpp )
SET_TARGET_PROPERTIES( test_module3 PROPERTIES OUTPUT_NAME test_module3 )

ADD_SHARED_LIBRARY( test_module4 test_class_4.cpp )
SET_TARGET_PROPERTIES( test_module4 PROPERTIES OUTPUT_NAME test_module4 )
