#pragma once

#include "define.hpp"
#include <string>
#include <chrono>
#include <unordered_map>
#include <vector>
#include <zmqpp/zmqpp.hpp>

SAIGON_NAMESPACE_BEGIN

class flcliapi
{
	class server
	{
	private:
		std::string endpoint_;
		bool alive_{};
		std::chrono::steady_clock::time_point ping_at_;
		std::chrono::steady_clock::time_point expiry_;

	public:
		server(std::string_view ep);
		void ping(zmqpp::socket_t* sock);
	};

	class agent
	{
	private:
		zmqpp::context_t& ctx_;
		zmqpp::socket* pipe_;
		zmqpp::socket router_; // talk to server

		std::unordered_map<std::string, flcliapi::server> servers_; // servers we've connected to
		std::vector<flcliapi::server> actives_;
		int seq_{}; // number of request ever sent
		std::unique_ptr<zmqpp::message_t> request_; // current request if any
		std::unique_ptr<zmqpp::message_t> reply_; // current reply if any
		std::chrono::steady_clock::time_point expiry_;

	public:
		agent(zmqpp::context_t& ctx, zmqpp::socket_t* pipe);
		void control_message();
	};
};

SAIGON_NAMESPACE_END