# using static library of zero mq
add_compile_definitions(ZMQ_STATIC ZMQPP_STATIC_DEFINE) ## or #define ZMQ_STATIC beofre include zmq.hpp
add_compile_definitions(DEBUG_ON) # for write debug spdlog

add_executable(mdworker
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"${CMAKE_SOURCE_DIR}/utils/zmqutil.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/worker/mdwrkapi.cpp"
	"worker/main.cpp"
 )
target_include_directories(mdworker PRIVATE ${SGLIB_INCLUDEDIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mdworker PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})