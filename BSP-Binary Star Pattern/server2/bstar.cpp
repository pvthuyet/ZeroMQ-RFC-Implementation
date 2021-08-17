#include "bstar.hpp"
#include "logger/logger_define.hpp"

SAIGON_NAMESPACE_BEGIN
bstar::bstar(zmqpp::context_t& ctx, std::string_view local, std::string_view remote, bool primary) :
	ctx_{ctx}
{
	state_ = primary ? state_t::STATE_PRIMARY : state_t::STATE_BACKUP;
	init(local, remote);
}

void bstar::start()
{
    update_peer_expiry();
    loop_->start();
}

void bstar::init(std::string_view local, std::string_view remote)
{
    LOGENTER;
    SPDLOG_INFO("{} - {}", local, remote);
	loop_ = std::make_unique<zmqpp::loop>();

	// Create publisher for state going to peer
	statepub_ = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::publish);
	statepub_->bind(local.data());

	// Create subscriber for state coming from peer
	statesub_ = std::make_unique<zmqpp::socket_t>(ctx_, zmqpp::socket_type::subscribe);
	statesub_->set(zmqpp::socket_option::subscribe, "");
	statesub_->connect(remote.data());

	// set-up basis reactor events
	loop_->add(std::chrono::milliseconds(STAR_HEARTBEAT), 0, [this]() -> bool {
		return this->send_state(this->statepub_.get());
		});

	loop_->add(*statesub_.get(), [this]() -> bool {
		return this->recv_state(statesub_.get());
		});
    LOGEXIT;
}

bool bstar::send_state(zmqpp::socket_t* sock)
{
    zmqpp::message_t msg;
    msg.push_back(static_cast<int>(state_));
    sock->send(msg);
    return true;
}

bool bstar::recv_state(zmqpp::socket_t* sock)
{
    zmqpp::message_t msg;
    sock->receive(msg);
    if (msg.parts()) {
        event_ = static_cast<event_t>(msg.get<int>(0));
        update_peer_expiry();
    }
	execute();
    return true;
}

void bstar::voter(std::string_view endpoint, zmqpp::socket_type type, Callable callback)
{
    LOGENTER;
    SPDLOG_INFO("Start listening {}", endpoint);
    frontend_host_ = endpoint;
	server_ = std::make_unique<zmqpp::socket_t>(ctx_, type);
	server_->bind(endpoint.data());
	loop_->add(*server_, [this, callback]() -> bool {
		// If server can accept input now
		this->event_ = event_t::CLIENT_REQUEST;
		if (this->execute()) {
			callback(this);
		}
		else {
			// Destroy waiting message
            SPDLOG_WARN("Server {} is not available", frontend_host_);
			zmqpp::message_t msg;
			server_->receive(msg);
		}
		return true;
		});
    LOGEXIT;
}

void bstar::new_active(Callable fn)
{
	active_fn_ = fn;
}

void bstar::new_passive(Callable fn)
{
	passive_fn_ = fn;
}

void bstar::update_peer_expiry()
{
    peer_expiry_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(STAR_HEARTBEAT * 3);
}

zmqpp::socket_t* bstar::get_frontend() const
{
    return server_.get();
}

std::string bstar::get_frontend_host() const
{
    return frontend_host_;
}

bool bstar::execute()
{
    bool rc = true;

    //  Primary server is waiting for peer to connect
    //  Accepts CLIENT_REQUEST events in this state_t
    if (state_ == state_t::STATE_PRIMARY) {
        if (event_ == event_t::PEER_BACKUP) {
            SPDLOG_INFO("Connected to backup (passive), ready active");
            state_ = state_t::STATE_ACTIVE;
            if (active_fn_) {
                active_fn_(this);
            }
        }
        else if (event_ == event_t::PEER_ACTIVE) {
            SPDLOG_INFO("Connected to backup (active), ready passive");
            state_ = state_t::STATE_PASSIVE;
            if (passive_fn_) {
                passive_fn_(this);
            }
        }
        else if (event_ == event_t::CLIENT_REQUEST) {
            // Allow client requests to turn us into the active if we've
            // waited sufficiently long to believe the backup is not
            // currently acting as active (i.e., after a failover)
            if (std::chrono::steady_clock::now() > peer_expiry_) {
                SPDLOG_INFO("Request from client, ready as active");
                state_ = state_t::STATE_ACTIVE;
                if (active_fn_) {
                    active_fn_(this);
                }
            }
            else
                // Don't respond to clients yet - it's possible we're
                // performing a failback and the backup is currently active
                rc = false;
        }
    }
    else if (state_ == state_t::STATE_BACKUP) {
        if (event_ == event_t::PEER_ACTIVE) {
            SPDLOG_INFO("Connected to primary (active), ready passive");
            state_ = state_t::STATE_PASSIVE;
            if (passive_fn_)
                passive_fn_(this);
        }
        else
            //  Reject client connections when acting as backup
            if (event_ == event_t::CLIENT_REQUEST)
                rc = false;
    }
    else {
        //  .split active and passive states
        //  These are the ACTIVE and PASSIVE states:
        if (state_ == state_t::STATE_ACTIVE) {
            if (event_ == event_t::PEER_ACTIVE) {
                //  Two actives would mean split-brain
                SPDLOG_INFO("Fatal error - dual actives, aborting");
                rc = false;
            }
        }
        else {
            //  Server is passive
            //  CLIENT_REQUEST events can trigger failover if peer looks dead
            if (state_ == state_t::STATE_PASSIVE) {
                if (event_ == event_t::PEER_PRIMARY) {
                    //  Peer is restarting - become active, peer will go passive
                    SPDLOG_INFO("Primary (passive) is restarting, ready active");
                    state_ = state_t::STATE_ACTIVE;
                }
                else if (event_ == event_t::PEER_BACKUP) {
                    //  Peer is restarting - become active, peer will go passive
                    SPDLOG_INFO("Backup (passive) is restarting, ready active");
                    state_ = state_t::STATE_ACTIVE;
                }
                else if (event_ == event_t::PEER_PASSIVE) {
                    //  Two passives would mean cluster would be non-responsive
                    SPDLOG_INFO("Fatal error - dual passives, aborting");
                    rc = false;
                }
                else if (event_ == event_t::CLIENT_REQUEST) {
                    //  Peer becomes active if timeout has passed
                    //  It's the client request that triggers the failover
                    if (std::chrono::steady_clock::now() > peer_expiry_) {
                        //  If peer is dead, switch to the active state_t
                        SPDLOG_INFO("Failover successful, ready active");
                        state_ = state_t::STATE_ACTIVE;
                    }
                    else
                        //  If peer is alive, reject connections
                        rc = false;

                    //  Call state_t change handler if necessary
                    if (state_ == state_t::STATE_ACTIVE && active_fn_)
                        active_fn_(this);
                }
            }
        }
    }
    return rc;
}
SAIGON_NAMESPACE_END