#include "mdwrkapi.hpp"
#include "mdp.h"
#include "logger/logger_define.hpp"
#include "utils/zmqutil.hpp"
#include "sgutils/random_factor.hpp"
#include <gsl/gsl_assert>
#include <algorithm>
#include <random>

SAIGON_NAMESPACE_BEGIN
mdwrkapi::mdwrkapi(zmqpp::context_t& ctx, 
	std::string_view broker, 
	std::string_view name,
	std::string_view id) :
	ctx_{ ctx },
	broker_{broker},
	service_name_{name},
	id_{id}
{}

zmqpp::context_t& mdwrkapi::get_context() const
{
	return ctx_;
}

std::string mdwrkapi::get_broker() const
{
	return broker_;
}

std::string mdwrkapi::get_service_name() const
{
	return service_name_;
}

std::string mdwrkapi::get_id() const
{
	return id_;
}

void mdwrkapi::connect_to_broker()
{
	// initialize worker
	worker_ = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::dealer);
	worker_->set(zmqpp::socket_option::linger, 0);
	std::string strid = id_;
	if (strid.empty()) {
		// Generate id number
		auto num = random_factor{}.random_number(1, 999);
		strid = std::format("{0:05d}", num);
	}
	worker_->set(zmqpp::socket_option::identity, strid);

	// connect to service with broker
	SPDLOG_INFO("Worker {} connecting to broker at {}", id_, broker_);
	worker_->connect(broker_);

	// register service with broker
	send_to_broker(MDPW_READY, service_name_, zmqpp::message_t{});

	// if liveness hits zero, queue is considered disconnected
	liveness_ = HEARTBEAT_LIVENESS;
	heartbeat_at_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(heartbeat_);
}

bool mdwrkapi::send(std::optional<zmqpp::message_t>&& msg)
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

std::optional<zmqpp::message_t> mdwrkapi::recv(std::stop_token tok)
{
	using clock = std::chrono::steady_clock;
	using milli = std::chrono::milliseconds;

	expect_reply_ = true;
	zmqpp::message_t msg;

	while (true) {
		if (tok.stop_requested()) {
			throw std::runtime_error("Recieved EXIT application command");
		}

		zmq_pollitem_t items[] = {
			{static_cast<void*>(*worker_.get()), 0, ZMQ_POLLIN, 0}
		};

		auto rc = zmq_poll(items, 1, HEARTBEAT_INTERVAL);
		if (rc < 0) {
			throw zmqpp::zmq_internal_exception{};
		}

		if (ZMQ_POLLIN & items[0].revents) {
			worker_->receive(msg);
			//SPDLOG_DEBUG("Received message from broker");
			//zmqutil::dump(msg);
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
				zmqutil::dump(msg);
				//  We should pop and save as many addresses as there are
				//  up to a null part, but for now, just save one...
				// Frame 3: Client address (envelope stack)
				reply_to_ = msg.get<std::string>(0);
				msg.pop_front();
				// Frame 4: Empty (zero bytes, envelope delimiter)
				Expects(msg.get<std::string>(0) == "");
				msg.pop_front();
				return std::make_optional<zmqpp::message_t>(std::move(msg)); // stop loop
			}
			else if (cmd == MDPW_HEARTBEAT) {
				liveness_ = HEARTBEAT_LIVENESS;
				//heartbeat_at_ = clock::now() + milli(heartbeat_);
			}
			else if (cmd == MDPW_DISCONNECT) {
				connect_to_broker();
			}
			else {
				SPDLOG_ERROR("Invalid input message {}", (int)(*cmd.c_str()));
				zmqutil::dump(msg);
			}
		}
		else if (--liveness_ == 0) {
			SPDLOG_WARN("Disconnected from broker - retrying...");
			std::this_thread::sleep_for(std::chrono::milliseconds(reconnect_));
			connect_to_broker();
		}

		// Send HEARTBEAT if it's time
		if (clock::now() > heartbeat_at_) {
			send_to_broker(MDPW_HEARTBEAT, "", zmqpp::message_t{});
			heartbeat_at_ = clock::now() + milli(heartbeat_);
		}
	}

	return std::nullopt;
}

bool mdwrkapi::send_to_broker(std::string_view command, 
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
	if (command == MDPW_REPLY) {
		SPDLOG_DEBUG("Sending {} to broker", mdps_commands[static_cast<int>(*command.data())]);
		zmqutil::dump(msg);
	}
	return worker_->send(msg);
}

SAIGON_NAMESPACE_END