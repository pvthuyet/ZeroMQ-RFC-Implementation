#pragma once

#include "define.hpp"
#include <vector>
#include <memory>
#include <zmqpp/zmqpp.hpp>

SAIGON_NAMESPACE_BEGIN
class config_parser;
class iworker;
class iworker_factory
{
public:
	virtual std::vector<std::unique_ptr<iworker>> create(zmqpp::context_t&, const config_parser&) = 0;
};
SAIGON_NAMESPACE_END