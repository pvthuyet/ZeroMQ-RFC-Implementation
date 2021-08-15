#include "binary_star_server1.hpp"
#include "logger/logger_define.hpp"
#include "gsl/gsl_assert"

SAIGON_NAMESPACE_BEGIN
binary_star_server1::binary_star_server1(zmqpp::context_t& ctx,
	std::string_view fehost,
	std::string_view statepub_host,
	std::string_view statesub_ep
	) :
	ctx_{ctx},
	frontend_host_{fehost},
	statepub_host_{statepub_host},
	statesub_ep_{statesub_ep}
{}

void binary_star_server1::start(bool isprimary)
{
	using clock = std::chrono::steady_clock;
	using namespace std::literals;
	using msec = std::chrono::milliseconds;
	
	try {
		// frontend socket
		zmqpp::socket_t frontend(ctx_, zmqpp::socket_type::router);
		frontend.bind(frontend_host_);

		// publish socket
		zmqpp::socket_t statepub(ctx_, zmqpp::socket_type::publish);
		statepub.bind(statepub_host_);

		// subscribe socket
		zmqpp::socket_t statesub(ctx_, zmqpp::socket_type::subscribe);
		statesub.set(zmqpp::socket_option::subscribe, "");
		statesub.connect(statesub_ep_);

		if (isprimary) {
			SPDLOG_INFO("Primary active, waiting for backup (passive)");
			SPDLOG_INFO("Publish primary host: {}, backup endpoint: {}", statepub_host_, statesub_ep_);
			fsm_.state_ = state_t::primary;
		}
		else {
			SPDLOG_INFO("Backup passive, waiting for primary (active)");
			SPDLOG_INFO("Publish backup host: {}, backup endpoint: {}", statepub_host_, statesub_ep_);
			fsm_.state_ = state_t::backup;
		}

		zmqpp::loop loop;
		auto send_state_at = clock::now() + msec(HEARTBEAT);
		auto tm_cb = [&send_state_at, &statepub, this]() -> bool {
			if (clock::now() > send_state_at) {
				zmqpp::message_t msg;
				msg << static_cast<int>(this->fsm_.state_);
				statepub.send(msg);
				send_state_at = clock::now() + msec(HEARTBEAT);
			}
			return true;
		};

		auto fe_cb = [&frontend, this]() -> bool {
			// Have a client request
			zmqpp::message_t msg;
			frontend.receive(msg);
			this->fsm_.event_ = event_t::client_request;
			if (false == this->update_state(this->fsm_)) {
				// Answer client by echoing request back
				frontend.send(msg);
			}
			return true;
		};

		auto sub_cb = [&statesub, this]() -> bool {
			// Have state from our peer, execute as event
			zmqpp::message_t msg;
			statesub.receive(msg);
			this->fsm_.event_ = static_cast<event_t>(msg.get<int>(0));
			if (this->update_state(this->fsm_)) {
				// Error, so exit
				throw std::runtime_error(std::format("Server state is wrong ({})", __LINE__));
			}
			this->fsm_.peer_exipry_ = clock::now() + msec(3*HEARTBEAT);
			return true;
		};

		loop.add(msec(HEARTBEAT), 0, tm_cb);
		loop.add(frontend, fe_cb);
		loop.add(statesub, sub_cb);
		loop.start();
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
	}
}

//void binary_star_server1::start(bool isprimary)
//{
//	using clock = std::chrono::steady_clock;
//	using namespace std::literals;
//	using msec = std::chrono::milliseconds;
//
//	try {
//		// frontend socket
//		zmqpp::socket_t frontend(ctx_, zmqpp::socket_type::router);
//		frontend.bind(frontend_host_);
//
//		// publish socket
//		zmqpp::socket_t statepub(ctx_, zmqpp::socket_type::publish);
//		statepub.bind(statepub_host_);
//
//		// subscribe socket
//		zmqpp::socket_t statesub(ctx_, zmqpp::socket_type::subscribe);
//		statesub.set(zmqpp::socket_option::subscribe, "");
//		statesub.connect(statesub_ep_);
//
//		if (isprimary) {
//			SPDLOG_INFO("Primary active, waiting for backup (passive)");
//			SPDLOG_INFO("Publish primary host: {}", statepub_host_);
//			SPDLOG_INFO("connect to backup endpoint: {}", statesub_ep_);
//			fsm_.state_ = state_t::primary;
//		}
//		else {
//			SPDLOG_INFO("Backup passive, waiting for primary (active)");
//			SPDLOG_INFO("Publish backup host: {}", statepub_host_);
//			SPDLOG_INFO("connect to primary endpoint: {}", statesub_ep_);
//			fsm_.state_ = state_t::backup;
//		}
//
//		zmqpp::poller_t poller;
//		poller.add(frontend);
//		poller.add(statesub);
//
//		auto send_state_at = clock::now() + msec(HEARTBEAT);
//		while (1) {
//			auto ok = poller.poll(HEARTBEAT);
//
//			if (poller.events(frontend) == zmqpp::poller_t::poll_in) {
//				
//			}
//
//			if (poller.events(statesub) == zmqpp::poller_t::poll_in) {
//
//				SPDLOG_INFO("Received msg from server");
//			}
//
//			if (clock::now() > send_state_at) {
//				statepub.send(std::format("{}", static_cast<int>(this->fsm_.state_)));
//				send_state_at = clock::now() + msec(HEARTBEAT);
//			}
//		}
//	}
//	catch (std::exception const& ex) {
//		SPDLOG_ERROR(ex.what());
//	}
//}

bool binary_star_server1::update_state(bstar_t& fsm)
{
	bool exception = false;
    //  These are the PRIMARY and BACKUP states; we're waiting to become
    //  ACTIVE or PASSIVE depending on events we get from our peer:
	switch (fsm.state_)
	{
	case state_t::primary:
	{
		if (event_t::peer_backup == fsm.event_) {
			SPDLOG_INFO("Connected to backup (passive), ready active");
			fsm.state_ = state_t::active;
		}
		else if (event_t::peer_active == fsm.event_) {
			SPDLOG_INFO("Connected to backup (active), ready passive");
			fsm.state_ = state_t::passive;
		}
		// Accept client connections
		break;
	}
	case state_t::backup:
	{
		if (event_t::peer_active == fsm.event_) {
			SPDLOG_INFO("Connected to primary (active), ready passive");
			fsm.state_ = state_t::passive;
		}
		// Reject client connections when acting as backup
		else if (fsm.event_ == event_t::client_request) {
			exception = true;
		}
		break;
	}
	case state_t::active:
	{
		if (event_t::peer_active == fsm.event_) {
			SPDLOG_ERROR("Fatal error - dual actives, aborting");
			exception = true;
		}
		break;
	}
	//  Server is passive
	//  CLIENT_REQUEST events can trigger failover if peer looks dead
	case state_t::passive:
	{
		switch (fsm.event_)
		{
		case event_t::peer_primary:
			SPDLOG_INFO("Primary (passive) is restarting, ready active");
			fsm.state_ = state_t::active;
			break;

		case event_t::peer_backup:
			SPDLOG_INFO("backup (passive) is restarting, ready active");
			fsm.state_ = state_t::active;
			break;

		case event_t::peer_passive:
			SPDLOG_ERROR("Fatal error - dual passives, aborting");
			exception = true;
			break;

		case event_t::client_request:
			// Peer becomes active if timeout has passed
			// It's the client request that triggers the failover
			if (std::chrono::steady_clock::now() > fsm.peer_exipry_) {
				// If peer is daed, switch to the active state
				SPDLOG_INFO("Failover successful, ready active");
				fsm.state_ = state_t::active;
			}
			else {
				exception = true;
			}
			break;
		}
		break;
	}
	default:
		break;
	}
	return exception;
}

SAIGON_NAMESPACE_END