#pragma once

#include "define.hpp"
#include <string>
#include <chrono>
#include <zmqpp/zmqpp.hpp>
#include "constant.hpp"

SAIGON_NAMESPACE_BEGIN
class mdwrkapi
{
private:
	zmqpp::context_t&	ctx_;
	std::string			broker_;
	std::string			service_name_;
	std::string			id_;
	std::unique_ptr<zmqpp::socket_t> worker_;

	// Heartbeat management
	std::chrono::steady_clock::time_point heartbeat_at_;	// when to send HEARTBEAT
	size_t	liveness_{};	// How many attempts left
	int		heartbeat_{ HEARTBEAT_INTERVAL };		// Heartbeat delay, msec
	int		reconnect_{ HEARTBEAT_INTERVAL };		// Reconnect delay, msec

	// Internal state
	bool expect_reply_{};	// Zero only at start

	// Return address, if any
	std::string reply_to_;

public:
	mdwrkapi(zmqpp::context_t& ctx,
		std::string_view broker, 
		std::string_view name,
		std::string_view id = "");

	zmqpp::context_t& get_context() const;
	std::string get_broker() const;
	std::string get_service_name() const;
	std::string get_id() const;

	void connect_to_broker();
	bool send(std::optional<zmqpp::message_t>&& msg);
	std::optional<zmqpp::message_t> recv(std::stop_token tok);

private:
	bool send_to_broker(std::string_view command, 
		std::string_view option,
		zmqpp::message_t&& msg
		);
};
SAIGON_NAMESPACE_END