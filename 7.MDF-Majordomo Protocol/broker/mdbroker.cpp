#include "mdbroker.hpp"
#include "utils/zmqutil.hpp"
#include "mdp.h"
#include "logger/logger_define.hpp"
#include "gsl/gsl_assert"
#include <deque>

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

//void mdbroker::run(std::stop_token tok)
//{
//	LOGENTER;
//	try {
//		zmqpp::loop loop{};
//
//		// timer
//		loop.add(std::chrono::milliseconds(HEARTBEAT_EXPIRY), 0, [this, tok]() -> bool {
//			//  Disconnect and delete any expired workers
//			//  Send heartbeats to idle workers if needed
//			// TODO
//			if (tok.stop_requested()) return false; // throw std::runtime_error("Stop request");
//			return true;
//			});
//
//		// socket
//		auto cbevent = [this](zmqpp::socket_t& sock) -> bool {
//			zmqpp::message_t msg;
//			sock.receive(msg);
//			SPDLOG_DEBUG("Received message");
//			zmqutil::dump(msg);
//
//			// identity of sender
//			auto sender = msg.get<std::string>(0);
//			msg.pop_front();
//
//			// Frame 0: empty
//			msg.pop_front();
//
//			// Frame 1: sender type : client or worker
//			auto header = msg.get<std::string>(0);
//			msg.pop_front();
//
//			if (header == MDPC_CLIENT) {
//				// TODO
//			}
//			else if (header == MDPW_WORKER) {
//				worker_process(sender, msg);
//			}
//			else {
//				SPDLOG_ERROR("Invalid message");
//			}
//			return true;
//		};
//
//		loop.add(*socket_, std::bind(cbevent, std::ref(*socket_)));
//
//		loop.start();
//	}
//	catch (std::exception const& ex) {
//		SPDLOG_ERROR(ex.what());
//	}
//	LOGEXIT;
//}

void mdbroker::run(std::stop_token tok)
{
	LOGENTER;
	using clock = std::chrono::steady_clock;
	using milli = std::chrono::milliseconds;
	try {
		zmqpp::poller_t poller;
		poller.add(*socket_);

		auto heartbeat_at = clock::now() + milli(HEARTBEAT_INTERVAL);

		while (!tok.stop_requested()) {
			poller.poll(HEARTBEAT_INTERVAL);

			if (poller.events(*socket_) == zmqpp::poller_t::poll_in) {
				zmqpp::message_t msg;
				socket_->receive(msg);
				//SPDLOG_DEBUG("Received message");
				//zmqutil::dump(msg);

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
			}

			//  Disconnect and delete any expired workers
			//  Send heartbeats to idle workers if needed
			if (clock::now() > heartbeat_at) {
				purge_workers();
				heartbeat_at = worker_send_heartbeat();
			}
		}
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

	if (cmd == MDPW_HEARTBEAT) {
		if (worker_ready) {
			wrk.update_expiry();
		}
		else {
			worker_delete(wrk.identity_, true);
		}
	}
	else if (cmd == MDPW_REPLY) {
		// TODO
	}
	else if (cmd == MDPW_READY) { //  Not first command in session
		if (worker_ready) {
			worker_delete(wrk.identity_, true);
		}
		else {
			// TODO .mmi
			auto srvname = msg.get<std::string>(0);
			wrk.service_name_ = srvname;
			auto& srv = service_require(srvname);
			srv.waiting_workers_.push_back(sender);
			SPDLOG_DEBUG("Worker '{} {}' joined", wrk.identity_, wrk.service_name_);
		}
	}
	else if (cmd == MDPW_DISCONNECT) {
		worker_delete(wrk.identity_, false);
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

//  Delete any idle workers that haven't pinged us in a while.
void mdbroker::purge_workers()
{
	auto now = std::chrono::steady_clock::now();
	std::deque<std::string> to_cull;

	// Collect all expiry workers;
	for (auto& [k, v] : workers_) {
		if (now > v.expiry_) {
			to_cull.push_back(v.identity_);
			SPDLOG_WARN("Worker '{} {}' is dead", v.identity_, v.service_name_);
		}
	}

	// Delete expiry workers
	for (auto& wrkid : to_cull) {
		worker_delete(wrkid, false);
	}
}

std::chrono::steady_clock::time_point mdbroker::worker_send_heartbeat()
{
	for (auto& [k, v] : services_) {
		for (auto const& wrkid : v.waiting_workers_) {
			worker_send(wrkid, MDPW_HEARTBEAT, "", zmqpp::message_t{});
		}
	}
	return std::chrono::steady_clock::now() + std::chrono::milliseconds(HEARTBEAT_INTERVAL);
}

void mdbroker::worker_delete(std::string wrkid, bool disconnect)
{
	Ensures(workers_.count(wrkid));
	if (disconnect) {
		worker_send(wrkid, MDPW_DISCONNECT, "", zmqpp::message_t{});
	}
	// Remove from service's waiting list
	for (auto& [k, v] : services_) {
		v.delete_waiting_worker(wrkid);
	}
	// Remove from worker list
	workers_.erase(wrkid);
}

void mdbroker::worker_send(std::string const& wrkid,
	std::string_view command, 
	std::string_view option, 
	zmqpp::message_t&& msg)
{
	Ensures(workers_.count(wrkid));
	if (option.size() > 0) {
		msg.push_front(option.data());
	}

	msg.push_front(command.data());
	msg.push_front(MDPW_WORKER);
	msg.push_front("");
	msg.push_front(wrkid);
	socket_->send(msg);
}

SAIGON_NAMESPACE_END