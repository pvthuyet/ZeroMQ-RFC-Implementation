#pragma once

#include "define.hpp"
#include <zmqpp/zmqpp.hpp>
#include "mdwrkapi.hpp"
#include <thread>

SAIGON_NAMESPACE_BEGIN
class mdworker
{
protected:
	std::string adminep_;
	mdwrkapi session_;
	std::unique_ptr<std::jthread> thread_;

public:
	mdworker(zmqpp::context_t& ctx,
		std::string_view brokerep,
		std::string_view adminep,
		std::string_view service_name,
		std::string_view identity
		);
	virtual ~mdworker() noexcept;

	void start();
	void wait();

private:
	virtual void run(std::stop_token tok) = 0;
};
SAIGON_NAMESPACE_END