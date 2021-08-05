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
	worker_queue queue_;
	std::unique_ptr<std::jthread> worker_;

public:
	ppbroker(zmqpp::context_t& ctx);
	~ppbroker() noexcept;

	void start(std::string const& fe, std::string const& be);
	void wait() noexcept;

private:
	void run(std::string fe, std::string be);
};
SAIGON_NAMESPACE_END