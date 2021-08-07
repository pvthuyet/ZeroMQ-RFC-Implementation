#pragma once

#include "define.hpp"
#include <zmqpp/zmqpp.hpp>
#include "mdwrkapi.hpp"
#include "interface/iworker.hpp"
#include <thread>

SAIGON_NAMESPACE_BEGIN
class mdworker : public iworker
{
protected:
	mdwrkapi session_;

public:
	mdworker(zmqpp::context_t& ctx,
		std::string_view brokerep,
		std::string_view adminep,
		std::string_view service_name = "",
		std::string_view identity = ""
		);
	virtual ~mdworker() noexcept;
};
SAIGON_NAMESPACE_END