# using static library of zero mq
add_compile_definitions(ZMQ_STATIC ZMQPP_STATIC_DEFINE) ## or #define ZMQ_STATIC beofre include zmq.hpp
add_compile_definitions(DEBUG_ON) # for write debug spdlog

## server
add_executable(flserver
	"${CMAKE_SOURCE_DIR}/logger/logger.hpp"
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"${CMAKE_SOURCE_DIR}/utils/config_parser.cpp"
	"${CMAKE_SOURCE_DIR}/utils/zmqutil.cpp"
	"server/flserver.cpp"
	"server/main.cpp"
)
target_include_directories(flserver PRIVATE 
	${SGLIB_INCLUDEDIR} 
	${CMAKE_SOURCE_DIR} 
	${CMAKE_CURRENT_SOURCE_DIR} 
	)
target_link_libraries(flserver PRIVATE 
	${WINDOWS_LIBRARIES} 
	${SGLIB_LIBRARIES})

## client
add_executable(flclient
	"${CMAKE_SOURCE_DIR}/logger/logger.hpp"
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"${CMAKE_SOURCE_DIR}/utils/config_parser.cpp"
	"${CMAKE_SOURCE_DIR}/utils/zmqutil.cpp"
	"client/flcliapi.cpp"
	"client/main.cpp"
)
target_include_directories(flclient PRIVATE 
	${SGLIB_INCLUDEDIR} 
	${CMAKE_SOURCE_DIR} 
	${CMAKE_CURRENT_SOURCE_DIR} 
	)
target_link_libraries(flclient PRIVATE 
	${WINDOWS_LIBRARIES} 
	${SGLIB_LIBRARIES})

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.json" "${CMAKE_CURRENT_BINARY_DIR}/config.json" COPYONLY)