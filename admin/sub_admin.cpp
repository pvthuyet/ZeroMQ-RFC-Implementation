#include "sub_admin.hpp"
#include <iostream>
#include "logger/logger_define.hpp"

SAIGON_NAMESPACE_BEGIN
sub_admin::sub_admin(zmqpp::context_t& ctx, std::string_view ep) :
	ctx_{ ctx },
	endpoint_{ ep }
{}

void sub_admin::wait()
{
	LOGENTER;
	using namespace std::string_literals;
	zmqpp::socket_t subscriber(ctx_, zmqpp::socket_type::subscribe);
	subscriber.connect(endpoint_);
	subscriber.set(zmqpp::socket_option::linger, 0);
	subscriber.set(zmqpp::socket_option::subscribe, "");
	while (true) {
		std::string msg;
		subscriber.receive(msg);
		if ("STOP"s == msg) {
			break;
		}
	}
	LOGEXIT;
}

SAIGON_NAMESPACE_END