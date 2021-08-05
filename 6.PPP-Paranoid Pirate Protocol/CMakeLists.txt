# using static library of zero mq
add_compile_definitions(ZMQ_STATIC ZMQPP_STATIC_DEFINE) ## or #define ZMQ_STATIC beofre include zmq.hpp
add_compile_definitions(DEBUG_ON) # for write debug spdlog

## broker
add_executable(ppbroker 
	"${CMAKE_SOURCE_DIR}/logger/logger.hpp"
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"constant.hpp"
	"broker/main.cpp" 
	"broker/worker_queue.hpp" 
	"broker/worker_queue.cpp" 
	"broker/ppbroker.hpp" 
	"broker/ppbroker.cpp")
target_include_directories(ppbroker PRIVATE ${SGLIB_INCLUDEDIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ppbroker PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})

## worker
add_executable(ppworker
	"${CMAKE_SOURCE_DIR}/logger/logger.hpp"
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"constant.hpp"
	"worker/main.cpp"
	"worker/ppworker.hpp"
	"worker/ppworker.cpp"
)
target_include_directories(ppworker PRIVATE ${SGLIB_INCLUDEDIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ppworker PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.json" "${CMAKE_CURRENT_BINARY_DIR}/config.json" COPYONLY)