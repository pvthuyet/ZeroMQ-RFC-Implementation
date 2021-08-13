# using static library of zero mq
add_compile_definitions(ZMQ_STATIC ZMQPP_STATIC_DEFINE) ## or #define ZMQ_STATIC beofre include zmq.hpp
add_compile_definitions(DEBUG_ON) # for write debug spdlog

## broker
add_executable(titanic
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"${CMAKE_SOURCE_DIR}/utils/zmqutil.cpp"
	"${CMAKE_SOURCE_DIR}/utils/config_parser.cpp"
	"${CMAKE_SOURCE_DIR}/interface/iworker.cpp"
	"${CMAKE_SOURCE_DIR}/admin/sub_admin.cpp"
	"${CMAKE_SOURCE_DIR}/7.MDF-Majordomo Protocol/worker/mdwrkapi.cpp"
	"${CMAKE_SOURCE_DIR}/7.MDF-Majordomo Protocol/worker/mdworker.cpp"
	"${CMAKE_SOURCE_DIR}/7.MDF-Majordomo Protocol/client/mdcliapi.cpp" 
	"titanic/titanic.cpp"
	"titanic/main.cpp"
 "titanic/request_worker.cpp"
 "titanic/reply_worker.cpp"
 "titanic/close_worker.cpp"
 )
target_include_directories(titanic PRIVATE 
${SGLIB_INCLUDEDIR} 
${CMAKE_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR} 
"${CMAKE_SOURCE_DIR}/7.MDF-Majordomo Protocol" 
"${CMAKE_SOURCE_DIR}/7.MDF-Majordomo Protocol/worker"
"${CMAKE_SOURCE_DIR}/7.MDF-Majordomo Protocol/client"
)
target_link_libraries(titanic PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})

add_executable(ticlient
	"${CMAKE_SOURCE_DIR}/logger/logger.cpp"
	"${CMAKE_SOURCE_DIR}/utils/zmqutil.cpp"
	"${CMAKE_SOURCE_DIR}/utils/config_parser.cpp"
	"${CMAKE_SOURCE_DIR}/7.MDF-Majordomo Protocol/client/mdcliapi.cpp" 
	"client/main.cpp"
	"client/ticlient.cpp"
	)
target_include_directories(ticlient PRIVATE ${SGLIB_INCLUDEDIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/7.MDF-Majordomo Protocol" "${CMAKE_SOURCE_DIR}/7.MDF-Majordomo Protocol/client")
target_link_libraries(ticlient PRIVATE ${WINDOWS_LIBRARIES} ${SGLIB_LIBRARIES})

configure_file("${CMAKE_SOURCE_DIR}/7.MDF-Majordomo Protocol/config.json" "${CMAKE_CURRENT_BINARY_DIR}/config.json" COPYONLY)