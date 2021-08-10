# using static library of zero mq
add_compile_definitions(ZMQ_STATIC ZMQPP_STATIC_DEFINE) ## or #define ZMQ_STATIC beofre include zmq.hpp
add_compile_definitions(DEBUG_ON) # for write debug spdlog

## worker
add_executable(mdworker
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"${CMAKE_SOURCE_DIR}/utils/zmqutil.cpp"
	"${CMAKE_SOURCE_DIR}/utils/config_parser.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/worker/mdwrkapi.cpp"
	"${CMAKE_SOURCE_DIR}/admin/sub_admin.cpp"
	"${CMAKE_SOURCE_DIR}/interface/iworker.cpp"
	"worker/main.cpp"
	"worker/mdworker.cpp"
	"worker/echoworker.cpp"
	"worker/mdworker_factory.cpp")
target_include_directories(mdworker PRIVATE ${SGLIB_INCLUDEDIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mdworker PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})

## broker
add_executable(mdbroker
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"${CMAKE_SOURCE_DIR}/utils/zmqutil.cpp"
	"${CMAKE_SOURCE_DIR}/utils/config_parser.cpp"
	"${CMAKE_SOURCE_DIR}/admin/sub_admin.cpp"
	"broker/main.cpp"	
 "broker/mdbroker.cpp")
target_include_directories(mdbroker PRIVATE ${SGLIB_INCLUDEDIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mdbroker PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})

## client
add_executable(mdclient
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"${CMAKE_SOURCE_DIR}/utils/zmqutil.cpp"
	"${CMAKE_SOURCE_DIR}/utils/config_parser.cpp"
	"client/main.cpp"
 "client/mdcliapi.hpp" "client/mdcliapi.cpp" )
target_include_directories(mdclient PRIVATE ${SGLIB_INCLUDEDIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mdclient PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})


# Config file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.json" "${CMAKE_CURRENT_BINARY_DIR}/config.json" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/run.bat" "${CMAKE_CURRENT_BINARY_DIR}/run.bat" COPYONLY)