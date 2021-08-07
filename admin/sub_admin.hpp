#pragma once
#include "define.hpp"
#include <zmqpp/zmqpp.hpp>
#include <thread>

SAIGON_NAMESPACE_BEGIN
class sub_admin
{
private:
	zmqpp::context_t& ctx_;
	std::string endpoint_;
	std::unique_ptr<std::jthread> thread_;

public:
	sub_admin(zmqpp::context_t& ctx, std::string_view ep);
	void start();
	void wait();

private:
	void run();
};

SAIGON_NAMESPACE_END