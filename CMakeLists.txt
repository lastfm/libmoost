PROJECT(moost)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(config.cmake)

ADD_LIBRARY(moost_core STATIC
            src/digest/base
            src/digest/sha2
            src/digest/rfc6234/sha1
            src/digest/rfc6234/sha224-256
            src/digest/rfc6234/sha384-512
            src/logging/global_scoped_logger
            src/logging/thrift
            src/service/posix_stream_stealer
            src/service/remote_shell
            src/service/appender
            src/timer
            )

ADD_LIBRARY(moost_configurable STATIC
            src/configurable/binder
            )

ADD_LIBRARY(moost_kvstore STATIC
            src/kvstore/kyoto_tycoon_connection
            )

ADD_LIBRARY(moost_mq STATIC
            src/mq/error_category
            src/mq/stream_manager
            src/mq/stomp_client
            src/mq/stomp_client_impl
            )

ADD_LIBRARY(moost_pdl STATIC
            src/pdl/dynamic_class
            src/pdl/dynamic_library
            src/pdl/dynamic_library_if
            src/pdl/symbol_function_cast
            )

ADD_LIBRARY(moost_signal STATIC
            src/signal/signal_handler
            )

ADD_LIBRARY(moost_mlog_nsca_appender SHARED
            src/logging/nsca_appender
           )

ADD_EXECUTABLE(mq-stomp-test-client
               src/tools/mq/stomp_test_client
              )

SET_TARGET_PROPERTIES(moost_mlog_nsca_appender PROPERTIES
                      SOVERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION})

TARGET_LINK_LIBRARIES(moost_mlog_nsca_appender
                      log4cxx
                      boost_regex
                      boost_system
                      pthread
                     )

TARGET_LINK_LIBRARIES(mq-stomp-test-client
                      moost_mq
                      moost_core
                      ${Boost_LIBRARIES}
                      ${Log4cxx_LIBRARIES}
                     )

INSTALL(TARGETS moost_core
                moost_configurable
                moost_kvstore
                moost_mq
                moost_pdl
                moost_signal
        ARCHIVE
        DESTINATION lib)

INSTALL(TARGETS moost_mlog_nsca_appender
        LIBRARY
        DESTINATION lib)

INSTALL(TARGETS mq-stomp-test-client
        DESTINATION bin)

INSTALL(DIRECTORY include/moost
        DESTINATION include)
