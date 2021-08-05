#pragma once

#include "define.hpp"
#include <zmqpp/zmqpp.hpp>
#include <memory>
#include <thread>

SAIGON_NAMESPACE_BEGIN
class lpclient
{
private:
	zmqpp::context_t& ctx_;
	std::string endpoint_;
	std::unique_ptr<zmqpp::socket_t> sock_;
	std::unique_ptr<std::jthread> thread_;

public:
	lpclient(zmqpp::context_t& ctx, std::string_view endpoint);
	~lpclient() noexcept;
	void start();
	void wait() noexcept;

private:
	void run();
	std::unique_ptr<zmqpp::socket_t> init_socket();
};
SAIGON_NAMESPACE_END