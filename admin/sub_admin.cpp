#include "sub_admin.hpp"
#include <iostream>
#include "logger/logger_define.hpp"
#include <boost/algorithm/string.hpp>

SAIGON_NAMESPACE_BEGIN
sub_admin::sub_admin(zmqpp::context_t& ctx, std::string_view ep) :
	ctx_{ ctx },
	endpoint_{ ep }
{}

void sub_admin::start()
{
	if (!thread_) {
		thread_ = std::make_unique<std::jthread>([this](std::stop_token tok) {
			this->run();
			});
	}
}

void sub_admin::wait()
{
	if (thread_ && thread_->joinable()) {
		thread_->join();
	}
}

void sub_admin::run()
{
	LOGENTER;
	using namespace std::string_literals;
	try {

		zmqpp::socket_t subscriber(ctx_, zmqpp::socket_type::subscribe);
		subscriber.connect(endpoint_);
		subscriber.set(zmqpp::socket_option::linger, 0);
		subscriber.set(zmqpp::socket_option::subscribe, "");
		while (true) {
			std::string msg;
			subscriber.receive(msg);
			if (boost::iequals("STOP"s, msg)) {
				break;
			}
		}
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
	}
	LOGEXIT;
}

SAIGON_NAMESPACE_END