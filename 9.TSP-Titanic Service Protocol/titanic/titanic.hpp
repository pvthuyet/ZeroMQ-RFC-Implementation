#pragma once

#include <sgutils/define.hpp>
#include <zmqpp/zmqpp.hpp>
#include "request_worker.hpp"

SAIGON_NAMESPACE_BEGIN
class titanic
{
private:
	zmqpp::context_t& ctx_;
	std::string brokerep_;
	std::string adminep_;

public:
	titanic(zmqpp::context_t& ctx,
		std::string_view brokerep,
		std::string_view adminep
	);

	void start();
	void wait();

private:
	bool request(zmqpp::socket_t* sock);
};
SAIGON_NAMESPACE_END