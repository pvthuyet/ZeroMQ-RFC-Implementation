#include "flserver.hpp"
#include "logger/logger_define.hpp"
#include "utils/zmqutil.hpp"
#include <memory>

SAIGON_NAMESPACE_BEGIN
flserver::flserver(zmqpp::context_t& ctx, std::string_view host, std::string_view ep)
	:
	ctx_{ctx},
	host_{host},
	endpoint_{ep}
{
}

void flserver::start()
{
	LOGENTER;
	try {
		auto socket = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::router);
		socket->set(zmqpp::socket_option::identity, endpoint_);
		socket->bind(host_);
		SPDLOG_INFO("Service is ready at {}", host_);
		while (true) {
			zmqpp::message_t msg;
			socket->receive(msg);
			zmqutil::dump(msg);
			if (msg.parts() == 0) break;

			//  Frame 0: identity of client
			//  Frame 1: PING, or client control frame
			//  Frame 2: request body
			auto identity = msg.get<std::string>(0);
			msg.pop_front();
			auto control = msg.get<std::string>(0);
			msg.pop_front();

			zmqpp::message_t reply;
			if (control == "PING") {
				reply.push_back("PONG");
			}
			else {
				reply.push_back(control);
				reply.push_back("OK");
			}
			reply.push_front(identity);
			zmqutil::dump(reply);
			socket->send(reply);
		}
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
	}
	LOGEXIT;
}

SAIGON_NAMESPACE_END