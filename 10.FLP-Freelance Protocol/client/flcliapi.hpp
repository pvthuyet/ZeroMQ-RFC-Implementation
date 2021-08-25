#pragma once

#include "define.hpp"
#include <string>
#include <chrono>
#include <unordered_map>
#include <queue>
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
		bool alive() const { return alive_; }
		void set_alive(bool v) { alive_ = v; }
		bool is_expiry() const { return std::chrono::steady_clock::now() > expiry_; }
		std::string get_endpoint() const { return endpoint_; }
		void reset_expiry();
	};

	class agent
	{
	private:
		zmqpp::context_t& ctx_;
		zmqpp::socket* pipe_;
		zmqpp::socket router_; // talk to server

		std::unordered_map<std::string, flcliapi::server> servers_; // servers we've connected to
		std::queue<std::string> actives_;
		int seq_{}; // number of request ever sent
		std::unique_ptr<zmqpp::message_t> request_{nullptr}; // current request if any
		std::chrono::steady_clock::time_point expiry_;

	public:
		agent(zmqpp::context_t& ctx, zmqpp::socket_t* pipe);

		void run();
		void control_message();
		void route_message();
		bool is_expiry() const { return std::chrono::steady_clock::now() > expiry_; }
	};

private:
	zmqpp::context_t& ctx_;
	std::unique_ptr<zmqpp::actor> freelancer_;

public:
	flcliapi(zmqpp::context_t& ctx);
	void connect(std::string_view ep);
	zmqpp::message_t request(zmqpp::message_t& msg);

private:
	bool run(zmqpp::socket_t* pipe);
};

SAIGON_NAMESPACE_END