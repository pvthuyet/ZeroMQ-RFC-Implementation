#pragma once
#include "mdworker.hpp"

SAIGON_NAMESPACE_BEGIN
class echoworker : public mdworker
{
public:
	inline static const std::string NAME = "echo";
public:
	echoworker(zmqpp::context_t& ctx,
		std::string_view brokerep,
		std::string_view adminep,
		std::string_view identity = ""
	);

private:
	void run(std::stop_token tok) override;
};
SAIGON_NAMESPACE_END