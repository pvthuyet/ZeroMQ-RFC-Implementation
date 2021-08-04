#pragma once

#include <thread>
#include "worker_queue.hpp"
#include <zmqpp/zmqpp.hpp>
#include "define.hpp"

SAIGON_NAMESPACE_BEGIN
class paranoid_pirate_broker
{
private:
	zmqpp::context_t& ctx_;
	worker_queue queue_;
	std::unique_ptr<std::jthread> worker_;

public:
	paranoid_pirate_broker(zmqpp::context_t& ctx);
	~paranoid_pirate_broker() noexcept;

	void start(std::string_view feport, std::string_view beport);
	void wait() noexcept;

private:
	void run(std::string_view feport, std::string_view beport);
};
SAIGON_NAMESPACE_END