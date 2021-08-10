#include "mdbroker.hpp"
#include "utils/zmqutil.hpp"
#include "mdp.h"
#include "logger/logger_define.hpp"
#include "gsl/gsl_assert"
#include <deque>

SAIGON_NAMESPACE_BEGIN

//--------------------------------------------------------------------------
void mdbroker::worker::update_expiry()
{
	expiry_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(HEARTBEAT_EXPIRY);
}

//--------------------------------------------------------------------------
void mdbroker::service::delete_waiting_worker(std::string const& wrkid)
{
	std::erase(waiting_workers_, wrkid);
}
void mdbroker::service::add_waiting_worker(const std::string& wrkid)
{
	waiting_workers_.push_back(wrkid);
}

void mdbroker::service::push_request(zmqpp::message_t&& req)
{
	requests_.push(std::move(req));
}

zmqpp::message_t mdbroker::service::pop_request()
{
	Ensures(requests_.size());
	zmqpp::message_t req{};
	std::swap(req, requests_.front());
	requests_.pop();
	return req;
}

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
		using clock = std::chrono::steady_clock;
		using milli = std::chrono::milliseconds;
	try {
		auto heartbeat_at = clock::now() + milli(HEARTBEAT_INTERVAL);
		zmqpp::loop loop{};

		// timer
		auto cbtimer = [this, tok, &heartbeat_at]() -> bool {
			//  Disconnect and delete any expired workers
			//  Send heartbeats to idle workers if needed
			// TODO
			if (tok.stop_requested()) return false; // stop looping

			//  Disconnect and delete any expired workers
			//  Send heartbeats to idle workers if needed
			if (clock::now() > heartbeat_at) {
				purge_workers();
				heartbeat_at = worker_send_heartbeat();
			}
			return true; // continue looping
		};

		// socket
		auto cbsock = [this](zmqpp::socket_t& sock) -> bool {
			zmqpp::message_t msg;
			sock.receive(msg);
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
				client_process(sender, msg);
			}
			else if (header == MDPW_WORKER) {
				worker_process(sender, msg);
			}
			else {
				SPDLOG_ERROR("Invalid message");
			}

			return true; // continue looping
		};

		loop.add(milli(HEARTBEAT_INTERVAL), 0, cbtimer);
		loop.add(*socket_, std::bind(cbsock, std::ref(*socket_)));
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

	bool isready = workers_.count(sender) > 0;
	auto& wrk = worker_require(sender);

	// Valid the command for this worker
	bool isnotvalid = (cmd == MDPW_HEARTBEAT && !isready) ||
		(cmd == MDPW_REPLY && !isready) ||
		(cmd == MDPW_READY && isready) ||
		(cmd == MDPW_READY && !isready && (0 == sender.find_first_of(MMI_KEY)));
	if (isnotvalid) {
		worker_delete(wrk.identity_, true);
		return;
	}

	if (cmd == MDPW_HEARTBEAT) {
		wrk.update_expiry();
	}
	else if (cmd == MDPW_REPLY) {
		//Frame 0: Empty frame
		//Frame 1 : “MDPW01”(six bytes, representing MDP / Worker v0.1)
		//Frame 2 : 0x03 (one byte, representing REPLY)
		//Frame 3 : Client address(envelope stack)
		//Frame 4 : Empty(zero bytes, envelope delimiter)
		//Frames 5 + : Reply body(opaque binary)

		//  Remove & save client return envelope and insert the
		//  protocol header and service name, then rewrap envelope.
		// Frame 3: client address
		auto cliaddr = msg.get<std::string>(0);
		msg.pop_front(); // Frame 3
		msg.pop_front(); // Frame 4

		// Build message send to client ------------------------------
		//Frame 0: Empty(zero bytes, invisible to REQ application)
		//Frame 1 : “MDPC01”(six bytes, representing MDP / Client v0.1)
		//Frame 2 : Service name(printable string)
		//Frames 3 + : Reply body(opaque binary)
		msg.push_front(wrk.service_name_);
		msg.push_front(MDPC_CLIENT);
		msg.push_front("");
		msg.push_front(cliaddr);
		zmqutil::dump(msg);
		socket_->send(msg); // send reply to client
		worker_waiting(wrk);
	}
	else if (cmd == MDPW_READY) { //  Not first command in session
		// TODO .mmi
		auto srvname = msg.get<std::string>(0);
		wrk.service_name_ = srvname;
		worker_waiting(wrk);
		SPDLOG_DEBUG("Worker '{}-{}' joined", wrk.identity_, wrk.service_name_);
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
			SPDLOG_WARN("Worker '{}-{}' is dead", v.identity_, v.service_name_);
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

void mdbroker::worker_waiting(mdbroker::worker& worker)
{
	worker.update_expiry();
	auto& service = service_require(worker.service_name_);
	service.add_waiting_worker(worker.identity_);
	// Attempt to process outstanding request
	service_dispatch(service, std::nullopt);
}

void mdbroker::service_dispatch(mdbroker::service& srv, std::optional<zmqpp::message_t>&& msg)
{
	if (msg) {
		srv.push_request(std::move(*msg));
	}

	purge_workers();
	while (!srv.waiting_workers_.empty() && !srv.requests_.empty()) {
		// Choose the most recently seen idle worker; others might be about to expire
		auto wrkid = std::min_element(std::cbegin(srv.waiting_workers_),
			std::cend(srv.waiting_workers_),
			[this](auto const& a, auto const& b) {
				return this->workers_.at(a).expiry_ < this->workers_.at(a).expiry_;
			}
		);
		
		auto req = srv.pop_request();
		worker_send(*wrkid, MDPW_REQUEST, "", std::move(req));
		srv.delete_waiting_worker(*wrkid);
	}
}

void mdbroker::service_internal(std::string_view service_name,
	std::optional<zmqpp::message_t>&& msg)
{
	// client id
	auto clientid = msg->get<std::string>(0);
	msg->pop_front();
	// Empty(zero bytes, envelope delimiter)
	msg->pop_front();
	if (service_name == MMI_SERVICE) {
		auto body = msg->get<std::string>(0);
		msg->pop_front();
		if (services_.count(body)) {
			msg->push_front(MMI_FOUND);
		}
		else {
			msg->push_front(MMI_NOT_FOUND);
		}
	}
	else {
		msg->push_front(MMI_NOT_IMPLEMENTED);
	}

	//  Remove & save client return envelope and insert the
	//  protocol header and service name, then rewrap envelope.
	msg->push_front(service_name.data());
	msg->push_front(MDPC_CLIENT);
	msg->push_front("");
	msg->push_front(clientid);
	socket_->send(*msg);
}

void mdbroker::client_process(std::string const& sender, zmqpp::message_t& msg)
{
	// ** Message format from Client
	//Frame 0: Empty(zero bytes, invisible to REQ application)
	//Frame 1 : “MDPC01”(six bytes, representing MDP / Client v0.1)
	//Frame 2 : Service name(printable string)
	//Frames 3 + : Request body(opaque binary)

	// ** Message format to worker
	//Frame 0: Empty frame
	//Frame 1 : “MDPW01”(six bytes, representing MDP / Worker v0.1)
	//Frame 2 : 0x02 (one byte, representing REQUEST)
	//Frame 3 : Client address(envelope stack)
	//Frame 4 : Empty(zero bytes, envelope delimiter)
	//Frames 5 + : Request body(opaque binary)

	// Service name and body
	Ensures(msg.parts() >= 2);
	// 
	auto service_name = msg.get<std::string>(0);
	msg.pop_front();

	auto& srv = service_require(service_name);
	// set  reply return address to client sender
	msg.push_front("");
	msg.push_front(sender);

	// TODO "mmi."
	if (0 == service_name.find_first_of(MMI_KEY)) {
		service_internal(service_name, std::make_optional<zmqpp::message_t>(std::move(msg)));
	}
	else {
		service_dispatch(srv, std::make_optional<zmqpp::message_t>(std::move(msg)));
	}
}

SAIGON_NAMESPACE_END