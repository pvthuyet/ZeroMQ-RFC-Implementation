#pragma once
#include "define.hpp"
#include <zmqpp/zmqpp.hpp>

SAIGON_NAMESPACE_BEGIN
class sub_admin
{
private:
	zmqpp::context_t& ctx_;
	std::string endpoint_;

public:
	sub_admin(zmqpp::context_t& ctx, std::string_view ep);
	void wait();
};

SAIGON_NAMESPACE_END