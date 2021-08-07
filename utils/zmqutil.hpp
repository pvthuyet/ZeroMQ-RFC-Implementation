#pragma once
#include "define.hpp"
#include <zmqpp/zmqpp.hpp>

SAIGON_NAMESPACE_BEGIN
namespace zmqutil
{
	void dump(const zmqpp::message_t& msg);
}
SAIGON_NAMESPACE_END