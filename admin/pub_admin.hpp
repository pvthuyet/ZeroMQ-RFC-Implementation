#pragma once
#include "define.hpp"
#include <zmqpp/zmqpp.hpp>

SAIGON_NAMESPACE_BEGIN
class pub_admin
{
private:
	zmqpp::context_t& ctx_;
	std::string endpoint_;

public:
	pub_admin(zmqpp::context_t& ctx, std::string_view ep);
	void start();
};

SAIGON_NAMESPACE_END