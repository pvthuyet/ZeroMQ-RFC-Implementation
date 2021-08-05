#pragma once

#include "define.hpp"
#include <zmqpp/zmqpp.hpp>
#include <memory>
#include <thread>

SAIGON_NAMESPACE_BEGIN
class ppworker
{
private:
	zmqpp::context_t& ctx_;
	std::string endpoint_;
	std::string adminep_;
	std::string identity_;
	std::unique_ptr<zmqpp::socket_t> sock_;
	std::unique_ptr<std::jthread> thread_;

public:
	ppworker(zmqpp::context_t& ctx, 
		std::string_view port, 
		std::string_view adminep,
		std::string_view identity);
	void start();
	void wait();

private:
	void run(std::stop_token tok);
	std::unique_ptr<zmqpp::socket_t> init_socket();
};
SAIGON_NAMESPACE_END