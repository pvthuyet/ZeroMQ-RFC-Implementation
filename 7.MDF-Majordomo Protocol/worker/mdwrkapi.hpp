#pragma once

#include "define.hpp"
#include <string>
#include <chrono>
#include <zmqpp/zmqpp.hpp>
#include "constant.hpp"

SAIGON_NAMESPACE_BEGIN
class mdwrk
{
private:
	zmqpp::context_t&	ctx_;
	std::string			broker_;
	std::string			service_;
	std::string			id_;
	std::unique_ptr<zmqpp::socket_t> worker_;

	// Heartbeat management
	std::chrono::steady_clock::time_point heartbeat_at_;	// when to send HEARTBEAT
	size_t	liveness_{};	// How many attempts left
	int		heartbeat_{HEARTBEAT_DELAY};		// Heartbeat delay, msec
	int		reconnect_{HEARTBEAT_DELAY};		// Reconnect delay, msec

	// Internal state
	bool expect_reply_{};	// Zero only at start

	// Return address, if any
	std::string reply_to_;

public:
	mdwrk(zmqpp::context_t& ctx, 
		std::string_view broker, 
		std::string_view service,
		std::string_view id);

	bool send(std::optional<zmqpp::message_t>&& msg);
	std::optional<zmqpp::message_t> recv();

private:
	void connect_to_broker();
	bool send_to_broker(std::string_view command, 
		std::string_view option,
		zmqpp::message_t&& msg
		);
};
SAIGON_NAMESPACE_END