#include "flcliapi.hpp"
#include "flp_constant.hpp"
#include "logger/logger_define.hpp"
#include "utils/zmqutil.hpp"
#include "gsl/gsl_assert"

SAIGON_NAMESPACE_BEGIN

using clock = std::chrono::steady_clock;
using milli = std::chrono::milliseconds;

flcliapi::server::server(std::string_view ep):
	endpoint_{ep}
{
	reset_expiry();
}

void flcliapi::server::ping(zmqpp::socket_t* sock)
{
	if (clock::now() > ping_at_) {
		zmqpp::message_t msg;
		msg.push_back(endpoint_);
		msg.push_back("PING");
		sock->send(msg);
		ping_at_ = clock::now() + milli(PING_INTERVAL);
	}
}

void flcliapi::server::reset_expiry()
{
	ping_at_ = clock::now() + milli(PING_INTERVAL);
	expiry_ = clock::now() + milli(SERVER_TTL);
}
//----------------------------------------------------------------------------------------

flcliapi::agent::agent(zmqpp::context_t& ctx, zmqpp::socket_t* pipe) :
	ctx_{ctx},
	pipe_{pipe},
	router_(ctx_, zmqpp::socket_type::router)
{
}

void flcliapi::agent::control_message()
{
	zmqpp::message_t msg;
	if (pipe_->receive(msg)) {
		auto cmd = msg.get<std::string>(0);
		msg.pop_front();

		if (cmd == COMMAND_CONNECT) {
			auto ep = msg.get<std::string>(0);
			SPDLOG_INFO("Connecting to {}", ep);
			router_.connect(ep);
			servers_.insert(std::make_pair(ep, server(ep)));
			actives_.push(ep);
		}
		else if (cmd == COMMAND_REQUEST) {
			// Current request must be nullptr
			Expects(!request_);
			msg.push_front(++seq_);
			request_ = std::make_unique<zmqpp::message_t>(std::move(msg));
			expiry_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(GLOBAL_TIMEOUT);
		}
	}
}

void flcliapi::agent::route_message()
{
	zmqpp::message_t msg;
	if (router_.receive(msg)) {
		//zmqutil::dump(msg);
		// Frame 0 is server that replied
		auto ep = msg.get<std::string>(0);
		msg.pop_front();

		auto& srv = servers_.at(ep);
		if (!srv.alive()) {
			srv.set_alive(true);
			actives_.push(ep);
		}
		srv.reset_expiry();

		// Frame 1 may be sequence number for reply
		if (msg.parts() > 1) {
			const auto seq = msg.get<int>(0);
			msg.pop_front();
			if (seq == seq_) {
				msg.push_front("OK");
				pipe_->send(msg);
				request_ = nullptr;
			}
		}
		//else {
		//	zmqutil::dump(msg);
		//}
	}
}

void flcliapi::agent::run()
{
	zmqpp::reactor react;
	
	react.add(*pipe_, [this]() {
		this->control_message();
		});

	react.add(router_, [this]() {
		this->route_message();
		});

	while (true) {
		react.poll(INTERVAL);

		// If we're processing a request, dispatch to next server
		if (request_) {
			if (is_expiry()) {
				// Request expired, kill it
				pipe_->send("FAILED");
				request_ = nullptr;
			}
			else {
				// Find server to talk to, remove any expired ones
				while (!actives_.empty()) {
					auto& srv = servers_.at(actives_.front());
					if (srv.is_expiry()) {
						actives_.pop();
						srv.set_alive(false);
					}
					else {
						zmqpp::message_t tmp;
						tmp.copy(*request_);
						tmp.push_front(srv.get_endpoint());
						SPDLOG_DEBUG("Sending request +++++++++++++++++++++++");
						zmqutil::dump(tmp);
						router_.send(tmp);
						break;
					}
				}
			}
		}

		// Send heartbeats to idle servers if needed
		for (auto& el : this->servers_) {
			el.second.ping(&this->router_);
		}
	}
}

//----------------------------------------------------------------------------------------

flcliapi::flcliapi(zmqpp::context_t& ctx):
	ctx_{ctx}
{
	freelancer_ = std::make_unique<zmqpp::actor>([this](zmqpp::socket_t* pipe) {
		return this->run(pipe);
		});
}

void flcliapi::connect(std::string_view ep)
{
	LOGENTER;
	zmqpp::message_t msg;
	msg << COMMAND_CONNECT;
	msg << ep.data();
	freelancer_->pipe()->send(msg);
	std::this_thread::sleep_for(milli(100)); // Allow connect to come up
	LOGEXIT;
}

zmqpp::message_t flcliapi::request(zmqpp::message_t& msg)
{
	msg.push_front(COMMAND_REQUEST);
	freelancer_->pipe()->send(msg);

	zmqpp::message_t reply;
	freelancer_->pipe()->receive(reply);
	SPDLOG_DEBUG("Received request --------------------------");
	zmqutil::dump(reply);
	return reply;
}

bool flcliapi::run(zmqpp::socket_t* pipe)
{
	pipe->send(zmqpp::signal::ok);
	agent agt(ctx_, pipe);
	agt.run();
	return true;
}
SAIGON_NAMESPACE_END