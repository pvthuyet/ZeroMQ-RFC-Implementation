# using static library of zero mq
add_compile_definitions(ZMQ_STATIC ZMQPP_STATIC_DEFINE) ## or #define ZMQ_STATIC beofre include zmq.hpp
add_compile_definitions(DEBUG_ON) # for write debug spdlog

## broker
add_executable(admin 
	"${CMAKE_SOURCE_DIR}/logger/logger.hpp"
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"main.cpp"
 "pub_admin.hpp" "pub_admin.cpp" "sub_admin.hpp" "sub_admin.cpp")
target_include_directories(admin PRIVATE ${SGLIB_INCLUDEDIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(admin PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})