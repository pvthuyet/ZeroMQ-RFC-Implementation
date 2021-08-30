#include "clonesrv.hpp"
#include "logger/logger_define.hpp"

SAIGON_NAMESPACE_BEGIN
clonesrv::clonesrv(zmqpp::context_t& ctx, bool primary)
	: ctx_{ctx},
	primary_{primary}
{
	if (primary_) {
		bstar_ = std::make_unique<bstar>(ctx_, "tcp://*:5003", "tcp://localhost:5004", primary_);
		bstar_->voter("tcp://*:5556", zmqpp::socket_type::router, [this](bstar* bs) -> bool {
			return this->snapshots(bs);
			});
		port_ = 5556;
		peer_ = 5566;
	}
	else {
		bstar_ = std::make_unique<bstar>(ctx_, "tcp://*:5004", "tcp://localhost:5003", primary_);
		bstar_->voter("tcp://*:5566", zmqpp::socket_type::router, [this](bstar* bs) -> bool {
			return this->snapshots(bs);
			});
		port_ = 5566;
		peer_ = 5556;
	}

	// Set up our clone server sockets
	publisher_ = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::publish);
	collector_ = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::subscribe);
	collector_->set(zmqpp::socket_option::subscribe, "");
	publisher_->bind(std::format("tcp://*:{}", port_ + 1));
	collector_->bind(std::format("tcp://*:{}", port_ + 2));

	// Set up our own clone client interface to peer
	subscriber_ = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::subscribe);
	subscriber_->set(zmqpp::socket_option::subscribe, "");
	subscriber_->connect(std::format("tcp://localhost:{}", peer_ + 1));
}

void clonesrv::run()
{
	
}

bool clonesrv::snapshots(bstar* bs)
{
	zmqpp::message_t msg;
	auto socket = bs->get_frontend();
	auto ok = socket->receive(msg);
	if (ok) {
		// Frame 0: identity
		// Frame 1: request 
		auto identity = msg.get<std::string>(0);
		auto request = msg.get<std::string>(1);
		std::string subtree;
		if (request == "ICANHAZ?") {
			subtree = msg.get<std::string>(2);
		}
		else {
			SPDLOG_ERROR("Bad request, abroting");
		}

		if (!subtree.empty()) {
			// Send state socket to client
			for (auto& [k, v] : kvmap_) {
				send_single(v, identity, subtree, *socket);
			}

			// Now send End message with get sequence number
			SPDLOG_INFO("Sending shapshot = {}", seq_);
			zmqpp::message_t reply;
			reply.push_back(identity);
			kvmsg kmsg(seq_);
			kmsg.set_key("KTHXBAI");
			kmsg.set_body(subtree);
			kmsg.send(*socket);
		}
	}
	return true;
}

void clonesrv::send_single(kvmsg& msg, std::string_view identity, std::string_view subtree, zmqpp::socket& socket)
{
	if (msg.get_key().starts_with(subtree)) {
		msg.send(socket, identity);
	}
}
SAIGON_NAMESPACE_END