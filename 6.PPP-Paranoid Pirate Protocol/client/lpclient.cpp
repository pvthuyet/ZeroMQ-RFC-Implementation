#include "lpclient.hpp"
#include "logger/logger_define.hpp"
#include "constant.hpp"

SAIGON_NAMESPACE_BEGIN
lpclient::lpclient(zmqpp::context_t& ctx, std::string_view endpoint) :
	ctx_{ ctx },
	endpoint_{ endpoint }
{}

lpclient::~lpclient() noexcept
{
	wait();
}

void lpclient::start()
{
	LOGENTER;
	if (thread_) {
		SPDLOG_ERROR("client is already started");
		return;
	}

	thread_ = std::make_unique<std::jthread>([this](std::stop_token tok) {
		this->run();
		});
	LOGEXIT;
}

void lpclient::wait() noexcept
{
	if (thread_ && thread_->joinable()) {
		thread_->join();
	}
}

void lpclient::run()
{
	LOGENTER;
	using namespace std::string_literals;
	using steady_clock = std::chrono::steady_clock;
	using milli = std::chrono::milliseconds;

	try {
		sock_ = init_socket();

		// If liveness hits zero, broker is considered disconnected
		int seq = 0;
		int retries = REQUEST_RETRIES;

		while (retries) {
			zmqpp::message_t req;
			req << ++seq;
			req << "hello";
			sock_->send(req);
			std::this_thread::sleep_for(milli(1));

			bool expect_reply = true;
			while (expect_reply) {
				zmq_pollitem_t items[] = {
					{static_cast<void*>(*sock_.get()), 0, ZMQ_POLLIN, 0}
				};

				auto rc = zmq_poll(items, 1, REQUEST_TIMEOUT);
				if (rc < 0) {
					throw zmqpp::zmq_internal_exception{};
				}

				if (items[0].revents & ZMQ_POLLIN) {
					// We got a reply from the server, must match seq
					zmqpp::message_t msg;
					sock_->receive(msg);
					auto msgid = msg.get<int>(0);
					auto body = msg.get<std::string>(1);
					if (msgid == seq) {
						SPDLOG_INFO("{} {}", body, msgid);
						retries = REQUEST_RETRIES;
						expect_reply = false;
					}
					else {
						SPDLOG_WARN("Malformed reply from server {}", msgid);
					}
				}
				else if (--retries == 0) {
					SPDLOG_ERROR("Server seems to be offline, abandoning");
					expect_reply = false;
					break;
				}
				else {
					SPDLOG_WARN("No response from server, retrying...");
					// Old socket will be confused; close it and open a new one
					sock_ = init_socket();
					// send request again
					zmqpp::message_t newMsg;
					newMsg << seq;
					sock_->send(newMsg);
				}
			}
		}
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
	}
	LOGEXIT;
}

std::unique_ptr<zmqpp::socket_t> lpclient::init_socket()
{
	using namespace std::string_literals;
	SPDLOG_INFO("Connecting to server {}", endpoint_);
	auto sock = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::request);
	sock->connect(endpoint_);
	// configure socket to not wait at close time
	sock->set(zmqpp::socket_option::linger, 0);

	return sock;
}

SAIGON_NAMESPACE_END