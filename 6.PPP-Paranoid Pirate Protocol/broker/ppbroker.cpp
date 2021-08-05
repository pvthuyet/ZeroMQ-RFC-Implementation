#include "ppbroker.hpp"
#include "logger/logger_define.hpp"
#include "constant.hpp"
#include "admin/sub_admin.hpp"

SAIGON_NAMESPACE_BEGIN
ppbroker::ppbroker(zmqpp::context_t& ctx, 
	std::string_view frontend_host,
	std::string_view backend_host,
	std::string_view adminep):
	ctx_{ctx},
	frontend_host_{ frontend_host },
	backend_host_{ backend_host },
	adminep_{adminep}
{}

void ppbroker::start()
{
	LOGENTER;
	if (worker_) {
		SPDLOG_ERROR("Broker is already started");
		return;
	}

	SPDLOG_INFO("Starting broker frontend {}, backend  {}", frontend_host_, backend_host_);
	worker_ = std::make_unique<std::jthread>([this](std::stop_token tok) {
		this->run(tok);
		});
	LOGEXIT;
}

void ppbroker::wait() noexcept
{
	// Subscribe the admin
	sg::sub_admin admin(ctx_, adminep_);
	admin.wait();
}

void ppbroker::run(std::stop_token tok)
{
	LOGENTER;
	using namespace std::string_literals;
	using steady_lock = std::chrono::steady_clock;

	try {
		zmqpp::socket_t frontend(ctx_, zmqpp::socket_type::router);
		zmqpp::socket_t backend(ctx_, zmqpp::socket_type::router);
		frontend.bind(frontend_host_);	// for clients
		backend.bind(backend_host_);	// for workers

		// queue of available workers
		auto heartbeat_at = steady_lock::now() + std::chrono::milliseconds(HEARTBEAT_INTERVAL);
		worker_queue queue;
		queue.reserve(10);

		while (!tok.stop_requested()) {
			zmq_pollitem_t items[] = {
				{backend, 0, ZMQ_POLLIN, 0},
				{frontend, 0, ZMQ_POLLIN, 0}
			};

			auto rc = zmq_poll(items, queue.size() ? 2 : 1, HEARTBEAT_INTERVAL);
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
					queue.push(identity);
				}
			}

			// Get client request, route to next worker
			if (ZMQ_POLLIN & items[1].revents) {
				zmqpp::message_t msg;
				frontend.receive(msg);
				if (queue.size()) {
					auto wrkident = queue.pop();
					msg.push_front(wrkident);
					backend.send(msg);
				}
				else {
					SPDLOG_ERROR("Received request from client but no worker available");
				}
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