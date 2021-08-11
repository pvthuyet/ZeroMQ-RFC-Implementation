#pragma once
#include "define.hpp"
#include <zmqpp/zmqpp.hpp>

SAIGON_NAMESPACE_BEGIN
namespace zmqutil
{
	void dump(const zmqpp::message_t& msg);
	void save(std::string_view path, const zmqpp::message_t& msg);
	zmqpp::message_t load(std::string_view path);
}
SAIGON_NAMESPACE_END