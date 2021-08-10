#include "mdcliapi.hpp"
#include "mdp.h"
#include "utils/zmqutil.hpp"
#include "sgutils/random_factor.hpp"
#include "logger/logger_define.hpp"
#include "gsl/gsl_assert"

SAIGON_NAMESPACE_BEGIN
mdcliapi::mdcliapi(zmqpp::context_t& ctx, 
	std::string_view broker,
	std::string_view id
) :
	ctx_{ctx},
	broker_{broker},
	id_{id}
{
	connect_to_broker();
}

void mdcliapi::connect_to_broker()
{
	socket_ = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::dealer);
	socket_->set(zmqpp::socket_option::linger, 0);
	std::string strid = id_;
	if (strid.empty()) {
		strid = std::format("{}",random_factor{}.random_number(1, 999));
	}
	socket_->set(zmqpp::socket_option::identity, strid);
	socket_->connect(broker_);
	SPDLOG_INFO("Client {} connected to {}", strid, broker_);
}

void mdcliapi::send(std::string_view service, std::string_view msg)
{
	//Frame 0: Empty(zero bytes, invisible to REQ application)
	//Frame 1 : “MDPC01”(six bytes, representing MDP / Client v0.1)
	//Frame 2 : Service name(printable string)
	//Frames 3 + : Request body(opaque binary)
	zmqpp::message_t req;
	req.push_back(service.data());
	req.push_back(msg.data());
	req.push_front(MDPC_CLIENT);
	req.push_front("");
	socket_->send(req);
}

std::string mdcliapi::recv()
{
	//Frame 0: Empty(zero bytes, invisible to REQ application)
	//Frame 1 : “MDPC01”(six bytes, representing MDP / Client v0.1)
	//Frame 2 : Service name(printable string)
	//Frames 3 + : Reply body(opaque binary)

	zmqpp::poller_t poller;
	poller.add(*socket_);
	auto isok = poller.poll(timeout_);
	if (poller.events(*socket_) == zmqpp::poller_t::poll_in) {
		zmqpp::message_t msg;
		socket_->receive(msg);
		Ensures(msg.parts() >= 4);
		msg.pop_front(); // Frame 0
		msg.pop_front(); // Frame 1
		msg.pop_front(); // Frame 2
		return msg.get< std::string>(0);
	}
	return {};
}

SAIGON_NAMESPACE_END