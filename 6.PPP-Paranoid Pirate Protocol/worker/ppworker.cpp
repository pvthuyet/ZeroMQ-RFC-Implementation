#include "ppworker.hpp"
#include "logger/logger_define.hpp"
#include "constant.hpp"
#include "admin/sub_admin.hpp"

SAIGON_NAMESPACE_BEGIN
ppworker::ppworker(zmqpp::context_t& ctx, 
	std::string_view endpoint, 
	std::string_view adminep, 
	std::string_view identity) :
	ctx_{ctx},
	endpoint_{ endpoint},
	adminep_{ adminep },
	identity_{identity}
{}

void ppworker::start()
{
	LOGENTER;
	if (thread_) {
		SPDLOG_ERROR("Worker is already started");
		return;
	}

	SPDLOG_INFO("Starting worker at {}", endpoint_);
	thread_ = std::make_unique<std::jthread>([this](std::stop_token tok) {
		this->run(tok);
		});
	LOGEXIT;
}

void ppworker::wait()
{
	// Subscribe the admin
	sg::sub_admin admin(ctx_, adminep_);
	admin.wait();
}

void ppworker::run(std::stop_token tok)
{
	LOGENTER;
	using namespace std::string_literals;
	using steady_clock = std::chrono::steady_clock;
	using milli = std::chrono::milliseconds;
	
	try {
		sock_ = init_socket();

		// If liveness hits zero, broker is considered disconnected
		size_t liveness = HEARTBEAT_LIVENESS;
		size_t interval = INTERVAL_INIT;

		// send out heartbeats at regular intervals
		auto heartbeat_at = steady_clock::now() + milli(HEARTBEAT_INTERVAL);

		while (!tok.stop_requested()) {
			zmq_pollitem_t items[] = {
				{static_cast<void*>(*sock_.get()), 0, ZMQ_POLLIN, 0}
			};

			auto rc = zmq_poll(items, 1, HEARTBEAT_INTERVAL);
			if (rc < 0) {
				throw zmqpp::zmq_internal_exception{};
			}

			if (items[0].revents & ZMQ_POLLIN) {
				interval = INTERVAL_INIT;
				// Get message
				// 3-part envelop + content -> request
				// 1-part "HEARTBEAT" -> heartbeat
				zmqpp::message_t msg;
				sock_->receive(msg);
				auto parts = msg.parts();

				if (msg.parts() == 1) {
					auto body = msg.get<std::string>(0);
					if (body == "HEARTBEAT"s) {
						liveness = HEARTBEAT_LIVENESS;
						//SPDLOG_DEBUG("Recevied HEARTBEAT");
					}
					else {
						SPDLOG_ERROR("{} {}: invalid message", identity_, body);
					}
				}
				else {
					sock_->send(msg);
					liveness = HEARTBEAT_LIVENESS;
				}
			}
			else if (--liveness == 0) {
				SPDLOG_WARN("{} heartbeat failure, can't reach broker. Reconnecting in {} msec...", identity_, interval);
				std::this_thread::sleep_for(milli(interval));

				if (interval < INTERVAL_MAX) {
					interval *= 2;
				}
				sock_ = init_socket();
				liveness = HEARTBEAT_LIVENESS;
			}

			// sned heartbeat to broker if it's time
			if (steady_clock::now() > heartbeat_at) {
				sock_->send("HEARTBEAT");
				heartbeat_at = steady_clock::now() + milli(HEARTBEAT_INTERVAL);
			}
		}
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
	}
	LOGEXIT;
}

std::unique_ptr<zmqpp::socket_t> ppworker::init_socket()
{
	using namespace std::string_literals;
	auto sock = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::dealer);
	// set random identity to make tracing easier
	sock->set(zmqpp::socket_option::identity, identity_);
	sock->connect(endpoint_);
	// configure socket to not wait at close time
	sock->set(zmqpp::socket_option::linger, 0);

	// Tell broker we're ready for work
	SPDLOG_INFO("{} worker ready", identity_);
	sock->send("READY"s);

	return sock;
}

SAIGON_NAMESPACE_END