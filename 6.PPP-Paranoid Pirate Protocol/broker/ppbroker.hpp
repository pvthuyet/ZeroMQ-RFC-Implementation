#pragma once

#include <thread>
#include "worker_queue.hpp"
#include <zmqpp/zmqpp.hpp>
#include "define.hpp"

SAIGON_NAMESPACE_BEGIN
class ppbroker
{
private:
	zmqpp::context_t& ctx_;
	std::string frontend_host_;
	std::string backend_host_;
	std::string adminep_;
	worker_queue queue_;
	std::unique_ptr<std::jthread> worker_;

public:
	ppbroker(zmqpp::context_t& ctx, 
		std::string_view frontend_host,
		std::string_view backend_host,
		std::string_view adminep);

	void start();
	void wait() noexcept;

private:
	void run(std::stop_token tok);
};
SAIGON_NAMESPACE_END