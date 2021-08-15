#include "bstarcli.hpp"
#include "bsp_constant.hpp"
#include "logger/logger_define.hpp"
#include "sgutils/random_factor.hpp"

SAIGON_NAMESPACE_BEGIN

bstarcli::bstarcli(zmqpp::context_t& ctx,
	std::string_view id) :
	ctx_{ctx},
	id_{id}
{}

void bstarcli::add_server(std::string_view addr)
{
	servers_.emplace_back(addr);
}

void bstarcli::connect_to_broker(std::string_view addr)
{
	socket_ = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::dealer);
	socket_->set(zmqpp::socket_option::linger, 0);
	std::string strid = id_;
	if (strid.empty()) {
		strid = std::format("{}", random_factor{}.random_number(1, 999));
	}
	socket_->set(zmqpp::socket_option::identity, strid);
	socket_->connect(addr.data());
	SPDLOG_INFO("Client {} connected to {}", strid, addr);
}

void bstarcli::start()
{
	LOGENTER;
	using namespace std::string_literals;
	using steady_clock = std::chrono::steady_clock;
	using milli = std::chrono::milliseconds;
	const int number_retries = 3;
	unsigned int srv_index = 0;

	try {
		connect_to_broker(servers_.at(srv_index % servers_.size()));
		// If liveness hits zero, broker is considered disconnected
		int seq = 0;
		int retries = number_retries;

		while (true) {
			std::this_thread::sleep_for(milli(1000));
			zmqpp::message_t req;
			req << ++seq;
			req << "hello";
			socket_->send(req);

			bool expect_reply = true;
			while (expect_reply) {
				zmq_pollitem_t items[] = {
					{static_cast<void*>(*socket_.get()), 0, ZMQ_POLLIN, 0}
				};

				auto rc = zmq_poll(items, 1, 1000);
				if (rc < 0) {
					throw zmqpp::zmq_internal_exception{};
				}

				if (items[0].revents & ZMQ_POLLIN) {
					// We got a reply from the server, must match seq
					zmqpp::message_t msg;
					socket_->receive(msg);
					auto msgid = msg.get<int>(0);
					auto body = msg.get<std::string>(1);
					if (msgid == seq) {
						SPDLOG_INFO("{} {}", body, msgid);
						retries = number_retries;
						expect_reply = false;
					}
					else {
						SPDLOG_WARN("Malformed reply from server {}", msgid);
					}
				}
				else if (--retries == 0) {
					SPDLOG_ERROR("Server seems to be offline, abandoning");
					expect_reply = false;
					srv_index++;
					retries = number_retries;
				}
				else {
					SPDLOG_WARN("No response from server, retrying...");
					// Old socket will be confused; close it and open a new one
					connect_to_broker(servers_.at(srv_index % servers_.size()));
					// send request again
					zmqpp::message_t newMsg;
					newMsg << seq;
					newMsg << "hello";
					socket_->send(newMsg);
				}
			}
		}
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
	}
	LOGEXIT;
}

SAIGON_NAMESPACE_END