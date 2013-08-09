add_executable(bq-continuous BufferQueueContinuousTest.cpp)
target_link_libraries(bq-continuous ${Boost_LIBRARIES})

add_executable(pull-bq-continuous PullBufferQueueContinuousTest.cpp)
target_link_libraries(pull-bq-continuous ${Boost_LIBRARIES})
