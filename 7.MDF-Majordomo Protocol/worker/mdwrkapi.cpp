#include "mdwrkapi.hpp"
#include "mdp.h"
#include "logger/logger_define.hpp"
#include "utils/zmqutil.hpp"
#include <gsl/gsl_assert>
#include <algorithm>

SAIGON_NAMESPACE_BEGIN
mdwrk::mdwrk(zmqpp::context_t& ctx, 
	std::string_view broker, 
	std::string_view service,
	std::string_view id) :
	ctx_{ ctx },
	broker_{broker},
	service_{service},
	id_{id}
{
	connect_to_broker();
}

void mdwrk::connect_to_broker()
{
	// initialize worker
	worker_ = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::dealer);
	worker_->set(zmqpp::socket_option::linger, 0);
	worker_->set(zmqpp::socket_option::identity, id_);

	// connect to service with broker
	SPDLOG_INFO("Connecting to broker at {}", broker_);
	worker_->connect(broker_);

	// register service with broker
	send_to_broker(MDPW_READY, service_, zmqpp::message_t{});

	// if liveness hits zero, queue is considered disconnected
	liveness_ = HEARTBEAT_LIVENESS;
	heartbeat_at_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(heartbeat_);
}

bool mdwrk::send(std::optional<zmqpp::message_t>&& msg)
{
	Ensures(msg || !expect_reply_);
	if (msg) {
		Ensures(!reply_to_.empty());
		msg->push_front("");			// Frame 4: Empty (zero bytes, envelope delimiter)
		msg->push_front(reply_to_);	// Frame 3: Client address (envelope stack)
		reply_to_.clear();
		return send_to_broker(MDPW_REPLY, "", std::move(*msg));
	}
	return false;
}

std::optional<zmqpp::message_t> mdwrk::recv()
{
	expect_reply_ = true;
	zmqpp::message_t msg;

	auto retry = [this]() -> bool {
		if (--liveness_ == 0) {
			SPDLOG_WARN("Disconnected from broker - retrying...");
			std::this_thread::sleep_for(std::chrono::milliseconds(reconnect_));
			connect_to_broker();
			return false;
		}

		// Send HEARTBEAT if it's time
		auto nw = std::chrono::steady_clock::now();
		if (std::chrono::steady_clock::now() > heartbeat_at_) {
			send_to_broker(MDPW_HEARTBEAT, "", zmqpp::message_t{});
			heartbeat_at_ += std::chrono::milliseconds(heartbeat_);
		}
		return true;
	};

	auto recevmsg = [this, &msg]() {
		worker_->receive(msg);
		SPDLOG_DEBUG("Received message from broker");
		zmqutil::dump(msg);
		liveness_ = HEARTBEAT_LIVENESS;

		// Make sure right format
		Expects(msg.parts() >= 3);

		// Frame 0: Empty
		Expects(msg.get<std::string>(0) == "");
		msg.pop_front();

		// Frame 1: worker version
		Expects(msg.get<std::string>(0) == MDPW_WORKER);
		msg.pop_front();

		// Frame 2: message command
		auto cmd = msg.get<std::string>(0);
		msg.pop_front();
		if (cmd == MDPW_REQUEST) {
			//  We should pop and save as many addresses as there are
			//  up to a null part, but for now, just save one...
			// Frame 3: Client address (envelope stack)
			reply_to_ = msg.get<std::string>(0);
			msg.pop_front();
			// Frame 4: Empty (zero bytes, envelope delimiter)
			Expects(msg.get<std::string>(0) == "");
			msg.pop_front();
			return false; // stop loop
		}
		else if (cmd == MDPW_HEARTBEAT) {
			// TODO: Do nothing for heartbeat
		}
		else if (cmd == MDPW_DISCONNECT) {
			connect_to_broker();
		}
		else {
			SPDLOG_ERROR("Invalid input message {}", (int)(*cmd.c_str()));
			zmqutil::dump(msg);
		}
		msg = zmqpp::message_t{};
		return true;
	};

	while(true){
		zmqpp::loop loop{};
		loop.add(std::chrono::milliseconds(heartbeat_), 0, retry);
		loop.add(*worker_.get(), recevmsg);
		loop.start();

		if (msg.parts()) break;
	}

	return std::make_optional<zmqpp::message_t>(std::move(msg));
}

bool mdwrk::send_to_broker(std::string_view command, 
	std::string_view option,
	zmqpp::message_t&& msg)
{
	// Stack protocal envelope to start of message
	if (!option.empty()) {
		msg.push_front(option.data()); // Frame 3: Service name (optional for READY command)
	}
	msg.push_front(command.data()); // Frame 2: mdps_command
	msg.push_front(MDPW_WORKER);	// Frame 1: Worker version
	msg.push_front("");				// Frame 0: Empty frame
	SPDLOG_DEBUG("Sending {} to broker", mdps_commands[static_cast<int>(*command.data())]);
	zmqutil::dump(msg);
	return worker_->send(msg);
}

SAIGON_NAMESPACE_END