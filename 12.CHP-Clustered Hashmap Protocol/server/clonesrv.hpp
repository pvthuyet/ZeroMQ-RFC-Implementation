#pragma once

#include <unordered_map>
#include <vector>
#include "define.hpp"
#include "kvmsg.hpp"
#include <zmqpp/zmqpp.hpp>
#include "BSP-Binary Star Pattern/server2/bstar.hpp"

SAIGON_NAMESPACE_BEGIN
class clonesrv
{
private:
	zmqpp::context_t& ctx_;
	std::unique_ptr<bstar> bstar_;
	std::unordered_map<std::string, kvmsg> kvmap_;
	size_t seq_;
	int port_{};
	int peer_{};
	std::unique_ptr<zmqpp::socket_t> publisher_;
	std::unique_ptr<zmqpp::socket_t> collector_;
	std::unique_ptr<zmqpp::socket_t> subscriber_;
	std::vector<kvmsg> pending_;
	bool primary_{};
	bool active_{};
	bool passive_{};

public:
	clonesrv(zmqpp::context_t& ctx, bool primary);
	void run();

private:
	bool snapshots(bstar* bs);
	void send_single(kvmsg& msg, std::string_view identity, std::string_view subtree, zmqpp::socket& socket);
};
SAIGON_NAMESPACE_END