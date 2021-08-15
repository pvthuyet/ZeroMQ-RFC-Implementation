# using static library of zero mq
add_compile_definitions(ZMQ_STATIC ZMQPP_STATIC_DEFINE) ## or #define ZMQ_STATIC beofre include zmq.hpp
add_compile_definitions(DEBUG_ON) # for write debug spdlog

## client
add_executable(bstarcli
	"${CMAKE_SOURCE_DIR}/logger/logger.hpp"
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"${CMAKE_SOURCE_DIR}/utils/config_parser.cpp"
	"client/bstarcli.cpp"
	"client/main.cpp"
)
target_include_directories(bstarcli PRIVATE 
	${SGLIB_INCLUDEDIR} 
	${CMAKE_SOURCE_DIR} 
	${CMAKE_CURRENT_SOURCE_DIR} 
	)
target_link_libraries(bstarcli PRIVATE 
	${WINDOWS_LIBRARIES} 
	${SGLIB_LIBRARIES})

## server
add_executable(bstarsrv1
	"${CMAKE_SOURCE_DIR}/logger/logger.hpp"
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"${CMAKE_SOURCE_DIR}/utils/config_parser.cpp"
	"server1/main.cpp"
	"server1/binary_star_server1.cpp"
 )
target_include_directories(bstarsrv1 PRIVATE ${SGLIB_INCLUDEDIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bstarsrv1 PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.json" "${CMAKE_CURRENT_BINARY_DIR}/config.json" COPYONLY)