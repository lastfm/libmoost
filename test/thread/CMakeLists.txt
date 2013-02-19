PROJECT(libmoost-thread-test)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(../../config.cmake)

ADD_EXECUTABLE(moost_thread_test
               async_batch_processor
               async_worker
               token_mutex
               threaded_job_scheduler
               main
               )

TARGET_LINK_LIBRARIES(moost_thread_test ${Boost_LIBRARIES})
