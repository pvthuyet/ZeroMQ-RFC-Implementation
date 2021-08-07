#pragma once

#include "define.hpp"
#include "interface/iworker_factory.hpp"

SAIGON_NAMESPACE_BEGIN
class mdworker_factory : public iworker_factory
{
public:
	std::vector<std::unique_ptr<iworker>> create(zmqpp::context_t&, const config_parser&) override;
};

SAIGON_NAMESPACE_END