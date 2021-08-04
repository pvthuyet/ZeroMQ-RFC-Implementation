# using static library of zero mq
add_compile_definitions(ZMQ_STATIC ZMQPP_STATIC_DEFINE) ## or #define ZMQ_STATIC beofre include zmq.hpp

# proxy
add_executable(ppbroker 
	"${CMAKE_SOURCE_DIR}/logger/logger.hpp"
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"constant.hpp"
	"ppbroker.cpp" 
	"worker_queue.hpp" 
	"worker_queue.cpp" 
	"paranoid_pirate_broker.hpp" 
	"paranoid_pirate_broker.cpp")
target_include_directories(ppbroker PRIVATE ${SGLIB_INCLUDEDIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(ppbroker PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})