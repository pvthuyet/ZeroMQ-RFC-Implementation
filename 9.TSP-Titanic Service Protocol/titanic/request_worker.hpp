#pragma once

#include <sgutils/define.hpp>
#include "mdworker.hpp"

SAIGON_NAMESPACE_BEGIN
class request_worker : public mdworker
{
	inline static const std::string NAME = "titanic.request";
	zmqpp::socket_t* pipe_;

public:
	request_worker(zmqpp::socket_t * pipe,
		zmqpp::context_t& ctx,
		std::string_view brokerep,
		std::string_view adminep,
		std::string_view identity = ""
	);

private:
	void run(std::stop_token tok) override;
};

SAIGON_NAMESPACE_END