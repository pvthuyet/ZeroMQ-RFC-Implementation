#pragma once

#include <sgutils/define.hpp>
#include <zmqpp/zmqpp.hpp>
#include "request_worker.hpp"

SAIGON_NAMESPACE_BEGIN
class titanic
{
private:
	zmqpp::context_t& ctx_;

public:
	titanic(zmqpp::context_t& ctx);

	void start();

private:
	bool request(zmqpp::socket_t* sock);
};
SAIGON_NAMESPACE_END