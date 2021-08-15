#pragma once

#include "define.hpp"
#include <zmqpp/zmqpp.hpp>
#include <string>
#include <memory>

SAIGON_NAMESPACE_BEGIN
class mdcliapi
{
private:
	zmqpp::context_t& ctx_;
	std::string broker_;
	std::string id_;
	std::unique_ptr<zmqpp::socket_t> socket_;
	int timeout_{3000};

public:
	mdcliapi(zmqpp::context_t& ctx, 
		std::string_view broker,
		std::string_view id = "");

	std::string get_server_endpoint() const;

	void send(std::string_view service, std::string_view msg);
	void send(std::string_view service, zmqpp::message_t& msg);

	zmqpp::message_t recv();

public:
	void connect_to_broker(std::string_view addr);
};
SAIGON_NAMESPACE_END