#include "pub_admin.hpp"
#include <iostream>
#include "logger/logger_define.hpp"

SAIGON_NAMESPACE_BEGIN
pub_admin::pub_admin(zmqpp::context_t& ctx, std::string_view ep):
	ctx_{ctx},
	endpoint_{ep}
{}

void pub_admin::start()
{
	LOGENTER;
	zmqpp::socket_t publisher(ctx_, zmqpp::socket_type::publish);
	publisher.bind(endpoint_);
	// send control message to subscribers
	std::string msg;
	std::cout << "Enter control message: ";
	std::cin >> msg;
	publisher.send(msg);
	LOGEXIT;
}

SAIGON_NAMESPACE_END