# using static library of zero mq
add_compile_definitions(ZMQ_STATIC ZMQPP_STATIC_DEFINE) ## or #define ZMQ_STATIC beofre include zmq.hpp
add_compile_definitions(DEBUG_ON) # for write debug spdlog

## server
add_executable(chserver
	"${CMAKE_SOURCE_DIR}/logger/logger.hpp"
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"${CMAKE_SOURCE_DIR}/utils/config_parser.cpp"
	"${CMAKE_SOURCE_DIR}/utils/zmqutil.cpp"
	"server/main.cpp"
 "kvmsg.cpp")
target_include_directories(chserver PRIVATE 
	${SGLIB_INCLUDEDIR} 
	${CMAKE_SOURCE_DIR} 
	${CMAKE_CURRENT_SOURCE_DIR} 
	)
target_link_libraries(chserver PRIVATE 
	${WINDOWS_LIBRARIES} 
	${SGLIB_LIBRARIES})

## client

## test
add_executable(test_kvmsg
	"kvmsg.cpp"
	"test/test_kvmsg.cpp"
	)
target_include_directories(test_kvmsg PRIVATE 
	${SGLIB_INCLUDEDIR} 
	${CMAKE_SOURCE_DIR} 
	${CMAKE_CURRENT_SOURCE_DIR} 
	)
target_link_libraries(test_kvmsg PRIVATE 
	${SGLIB_LIBRARIES}
	GTest::gtest_main
	)
enable_testing()
add_test(
	NAME google_test
	COMMAND $<TARGET_FILE:test_kvmsg>
)