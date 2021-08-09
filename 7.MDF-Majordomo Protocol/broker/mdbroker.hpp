#pragma once
#include "define.hpp"
#include "constant.hpp"
#include <zmqpp/zmqpp.hpp>
#include "admin/sub_admin.hpp"
#include <string>
#include <vector>
#include <unordered_map>

SAIGON_NAMESPACE_BEGIN
class mdbroker
{
	struct worker
	{
		std::string identity_;
		std::string service_name_;
		std::chrono::steady_clock::time_point expiry_;
		void update_expiry()
		{
			expiry_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(HEARTBEAT_EXPIRY);
		}
	};

	struct service
	{
		std::string name_;
		std::vector<std::string> waiting_workers_;
		void delete_waiting_worker(std::string const& wrkid)
		{
			std::erase(waiting_workers_, wrkid);
		}
	};

private:
	zmqpp::context_t& ctx_;
	std::string host_;
	sub_admin admin_;
	std::unique_ptr<zmqpp::socket_t> socket_;
	std::unordered_map<std::string, service> services_;
	std::unordered_map<std::string, worker> workers_;
	std::unique_ptr<std::jthread> thread_;

public:
	mdbroker(zmqpp::context_t& ctx, 
		std::string_view host,
		std::string_view adminep
		);
	~mdbroker() noexcept;

	void start();
	void wait();

private:
	void run(std::stop_token tok);
	void worker_process(std::string const& sender, zmqpp::message_t& msg);
	mdbroker::worker& worker_require(std::string const& sender);
	mdbroker::service& service_require(std::string const& name);

	void purge_workers();
	std::chrono::steady_clock::time_point worker_send_heartbeat();
	void worker_delete(std::string wrkid, bool disconnect);
	void worker_send(std::string const& wrkid,
		std::string_view command, 
		std::string_view option, 
		zmqpp::message_t&& msg);
};
SAIGON_NAMESPACE_END