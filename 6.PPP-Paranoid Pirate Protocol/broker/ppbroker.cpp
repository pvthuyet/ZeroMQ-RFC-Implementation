#include "ppbroker.hpp"
#include "logger/logger_define.hpp"
#include "constant.hpp"

SAIGON_NAMESPACE_BEGIN
ppbroker::ppbroker(zmqpp::context_t& ctx):
	ctx_{ctx}
{}

ppbroker::~ppbroker() noexcept
{
	wait();
}

void ppbroker::start(std::string_view epfrontend, std::string_view epbackend)
{
	LOGENTER;
	if (worker_) {
		SPDLOG_ERROR("Broker is already started");
		return;
	}

	SPDLOG_INFO("Starting broker frontend port {}, backend port {}", epfrontend, epbackend);
	worker_ = std::make_unique<std::jthread>([this, epfrontend, epbackend](std::stop_token tok) {
		this->run(epfrontend, epbackend);
		});
	LOGEXIT;
}

void ppbroker::wait() noexcept
{
	if (worker_ && worker_->joinable()) {
		worker_->join();
	}
}

void ppbroker::run(std::string_view epfrontend, std::string_view epbackend)
{
	LOGENTER;
	using namespace std::string_literals;
	using steady_lock = std::chrono::steady_clock;

	try {
		zmqpp::socket_t frontend(ctx_, zmqpp::socket_type::router);
		zmqpp::socket_t backend(ctx_, zmqpp::socket_type::router);
		frontend.bind(epfrontend.data());	// for clients
		backend.bind(epbackend.data());	// for workers

		// queue of available workers
		auto heartbeat_at = steady_lock::now() + std::chrono::milliseconds(HEARTBEAT_INTERVAL);
		worker_queue queue;

		zmq_pollitem_t items[] = {
			{backend, 0, ZMQ_POLLIN, 0},
			{frontend, 0, ZMQ_POLLIN, 0}
		};
		int rc{};
		while (true) {
			if (queue.size()) {
				rc = zmq_poll(items, 2, HEARTBEAT_INTERVAL);
			}
			else {
				rc = zmq_poll(items, 1, HEARTBEAT_INTERVAL);
			}

			if (rc < 0) {
				throw zmqpp::zmq_internal_exception{};
			}

			// Handle worker activity on backend
			if (ZMQ_POLLIN & items[0].revents) {
				zmqpp::message_t msg;
				backend.receive(msg);
				auto identity = msg.get<std::string>(0);

				// this is control message
				if (msg.parts() == 2) {
					auto ctrl = msg.get<std::string>(1);
					if ("READY"s == ctrl) {
						queue.erase(identity);
						queue.push(identity);
					}
					else {
						if ("HEARTBEAT"s == ctrl) {
							queue.refresh(identity);
							SPDLOG_DEBUG("Recevied HEARTBEAT");
						}
						else {
							SPDLOG_ERROR("Invalid message from {} - {}", identity, ctrl);
						}
					}
				}
				else {
					// forward message to frontend
					// remove worker identity
					msg.pop_front();
					frontend.send(msg);
				}
			}

			// Get client request, route to next worker
			if (ZMQ_POLLIN & items[1].revents) {
				zmqpp::message_t msg;
				frontend.receive(msg);
				auto wrkident = queue.pop();
				msg.push_front(wrkident);
				backend.send(msg);
			}

			// send out heartbeats at regular intervals
			if (steady_lock::now() > heartbeat_at) {
				for (auto& w : queue.data()) {
					zmqpp::message_t msg;
					msg << w.identity_;
					msg << "HEARTBEAT";
					backend.send(msg);
				}
				heartbeat_at = steady_lock::now() + std::chrono::milliseconds(HEARTBEAT_INTERVAL);
			}
			queue.purge();
		}
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
	}

	LOGEXIT;
}

SAIGON_NAMESPACE_END