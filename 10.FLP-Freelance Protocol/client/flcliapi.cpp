#include "flcliapi.hpp"
#include "flp_constant.hpp"
#include "logger/logger_define.hpp"
#include "gsl/gsl_assert"

SAIGON_NAMESPACE_BEGIN
flcliapi::server::server(std::string_view ep):
	endpoint_{ep}
{
	ping_at_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(PING_INTERVAL);
	expiry_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(SERVER_TTL);
}

void flcliapi::server::ping(zmqpp::socket_t* sock)
{
	using clock = std::chrono::steady_clock;
	using milli = std::chrono::milliseconds;
	if (clock::now() > ping_at_) {
		zmqpp::message_t msg;
		msg.push_back(endpoint_);
		msg.push_back("PING");
		sock->send(msg);
		ping_at_ = clock::now() + milli(PING_INTERVAL);
	}
}
//----------------------------------------------------------------------------------------

flcliapi::agent::agent(zmqpp::context_t& ctx, zmqpp::socket_t* pipe) :
	ctx_{ctx},
	pipe_{pipe},
	router_(ctx_, zmqpp::socket_type::router)
{
}

void flcliapi::agent::control_message()
{
	zmqpp::message_t msg;
	if (pipe_->receive(msg)) {
		auto cmd = msg.get<std::string>(0);
		msg.pop_front();

		if (cmd == COMMAND_CONNECT) {
			auto ep = msg.get<std::string>(0);
			SPDLOG_INFO("Connecting to {}", ep);
			router_.connect(ep);
			servers_.insert(std::make_pair(ep, server(ep)));
			actives_.push_back(servers_.at(ep));
		}
		else if (cmd == COMMAND_REQUEST) {
			// Current request must be nullptr
			Expects(request_);
			auto seqstr = std::format("{}", ++seq_);
			msg.push_front(seqstr);
			request_ = std::make_unique<zmqpp::message_t>(std::move(msg));
			expiry_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(GLOBAL_TIMEOUT);
		}
	}
}

SAIGON_NAMESPACE_END