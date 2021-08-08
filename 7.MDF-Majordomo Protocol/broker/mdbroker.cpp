#include "mdbroker.hpp"
#include "utils/zmqutil.hpp"
#include "mdp.h"
#include "logger/logger_define.hpp"

SAIGON_NAMESPACE_BEGIN
mdbroker::mdbroker(zmqpp::context_t& ctx, 
	std::string_view host,
	std::string_view adminep):
	ctx_{ctx},
	host_{host},
	admin_(ctx_, adminep)
{}

mdbroker::~mdbroker() noexcept = default;

void mdbroker::start()
{
	LOGENTER;
	if (thread_) {
		SPDLOG_ERROR("Broker is already started");
		return;
	}

	SPDLOG_INFO("Starting broker...");
	thread_ = std::make_unique<std::jthread>([this](std::stop_token tok) {
		this->socket_ = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::router);
		this->socket_->bind(host_);
		this->run(tok);
		});
	admin_.start();

	LOGEXIT;
}

void mdbroker::wait()
{
	admin_.wait();
}

void mdbroker::run(std::stop_token tok)
{
	LOGENTER;
	try {
		zmqpp::loop loop{};

		// timer
		loop.add(std::chrono::milliseconds(HEARTBEAT_INTERVAL), 0, [this, tok]() -> bool {
			//  Disconnect and delete any expired workers
			//  Send heartbeats to idle workers if needed
			// TODO
			if (tok.stop_requested()) throw std::runtime_error("Stop request");
			return true;
			});

		// socket
		auto cbevent = [this, tok](zmqpp::socket_t& sock) -> bool {
			if (tok.stop_requested()) throw std::runtime_error("Stop request");

			zmqpp::message_t msg;
			sock.receive(msg);
			SPDLOG_DEBUG("Received message");
			zmqutil::dump(msg);

			// identity of sender
			auto sender = msg.get<std::string>(0);
			msg.pop_front();

			// Frame 0: empty
			msg.pop_front();

			// Frame 1: sender type : client or worker
			auto header = msg.get<std::string>(0);
			msg.pop_front();

			if (header == MDPC_CLIENT) {
				// TODO
			}
			else if (header == MDPW_WORKER) {
				worker_process(sender, msg);
			}
			else {
				SPDLOG_ERROR("Invalid message");
			}
			return true;
		};

		loop.add(*socket_, std::bind(cbevent, std::ref(*socket_)));

		loop.start();
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
	}
	LOGEXIT;
}

void mdbroker::worker_process(std::string const& sender, zmqpp::message_t& msg)
{
	// Frame 2: command
	auto cmd = msg.get<std::string>(0);
	msg.pop_front();

	bool worker_ready = workers_.count(sender) > 0;

	auto& wrk = worker_require(sender);
	if (cmd == MDPW_READY) {
		if (worker_ready) {
			// TODO remove old one
		}
		else {
			// TODO .mmi
			auto srvname = msg.get<std::string>(0);
			wrk.service_name_ = srvname;
			auto& srv = service_require(srvname);
			srv.waiting_workers_.push_back(sender);
		}
	}
	else if (cmd == MDPW_HEARTBEAT) {
		if (worker_ready) {
			wrk.update_expiry();
		}
		else {
			// TODO delete
		}
	}
	else if (cmd == MDPW_DISCONNECT) {
		// TODO
	}
	else if (cmd == MDPW_REPLY) {
		// TODO
	}
	else {
		SPDLOG_ERROR("Invalid input message ({})", (int)*cmd.c_str());
		zmqutil::dump(msg);
	}
}

mdbroker::worker& mdbroker::worker_require(std::string const& sender)
{
	if (!workers_.count(sender)) {
		mdbroker::worker wrk{
			.identity_ = sender,
			.expiry_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(HEARTBEAT_EXPIRY)
		};
		workers_.insert(std::make_pair(sender, std::move(wrk)));
	}

	return workers_.at(sender);
}

mdbroker::service& mdbroker::service_require(std::string const& name)
{
	if (!services_.count(name)) {
		mdbroker::service srv{
			.name_ = name
		};
		services_.insert(std::make_pair(name, std::move(srv)));
	}

	return services_.at(name);
}

SAIGON_NAMESPACE_END