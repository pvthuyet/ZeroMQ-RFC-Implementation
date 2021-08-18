#pragma once
#include "define.hpp"
#include <zmqpp/zmqpp.hpp>
#include <string>

SAIGON_NAMESPACE_BEGIN
class flserver
{
private:
	zmqpp::context_t& ctx_;
	std::string host_;
	std::string endpoint_;

public:
	flserver(zmqpp::context_t& ctx, std::string_view host, std::string_view ep);
	void start();
};
SAIGON_NAMESPACE_END