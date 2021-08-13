#pragma once

#include <sgutils/define.hpp>
#include "mdworker.hpp"

SAIGON_NAMESPACE_BEGIN
class reply_worker : public mdworker
{
	inline static const std::string NAME = "titanic.reply";

public:
	reply_worker(zmqpp::context_t& ctx,
		std::string_view brokerep,
		std::string_view adminep,
		std::string_view identity = ""
	);

private:
	void run(std::stop_token tok) override;
};

SAIGON_NAMESPACE_END