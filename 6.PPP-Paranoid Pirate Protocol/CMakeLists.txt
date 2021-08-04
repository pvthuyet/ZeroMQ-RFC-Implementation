# using static library of zero mq
add_compile_definitions(ZMQ_STATIC ZMQPP_STATIC_DEFINE) ## or #define ZMQ_STATIC beofre include zmq.hpp

# dechat
add_executable(ppqueue 
	"${CMAKE_SOURCE_DIR}/logger/logger.hpp"
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"ppproxy.cpp" 
	"worker_queue.hpp" 
	"worker_queue.cpp" 
	"paranoid_pirate_proxy.hpp" 
	"paranoid_pirate_proxy.cpp")
target_include_directories(ppqueue PRIVATE ${SGLIB_INCLUDEDIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(ppqueue PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})