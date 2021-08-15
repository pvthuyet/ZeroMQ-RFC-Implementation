#pragma once

#include "define.hpp"
#include <zmqpp/zmqpp.hpp>

SAIGON_NAMESPACE_BEGIN
class bstarcli
{
private:
	zmqpp::context_t& ctx_;
	std::string id_;
	std::vector<std::string> servers_;
	std::unique_ptr<zmqpp::socket_t> socket_;

public:
	bstarcli(zmqpp::context_t& ctx,
		std::string_view id = "");

	void add_server(std::string_view addr);
	void start();

private:
	void connect_to_broker(std::string_view addr);
};
SAIGON_NAMESPACE_END