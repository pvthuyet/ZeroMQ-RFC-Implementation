# using static library of zero mq
add_compile_definitions(ZMQ_STATIC ZMQPP_STATIC_DEFINE) ## or #define ZMQ_STATIC beofre include zmq.hpp

# dechat
add_executable(ppqueue "ppqueue.cpp" "worker_queue.hpp" "worker_queue.cpp")
target_include_directories(ppqueue PRIVATE ${SGLIB_INCLUDEDIR})
target_link_libraries(ppqueue PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})