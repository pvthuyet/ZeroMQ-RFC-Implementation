#pragma once

#include "define.hpp"
#include "mdcliapi.hpp"

SAIGON_NAMESPACE_BEGIN
class ticlient
{
private:
	mdcliapi client_;

public:
	ticlient(zmqpp::context_t& ctx,
		std::string_view broker,
		std::string_view id = "");

	std::string send(std::string_view srvname, zmqpp::message_t& req);
	zmqpp::message_t recv(std::string_view uuid);

private:
	zmqpp::message_t service_call(std::string_view srvname, zmqpp::message_t& req);
};
SAIGON_NAMESPACE_END