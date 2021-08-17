#pragma once

#include "define.hpp"
#include <zmqpp/zmqpp.hpp>
#include <chrono>
#include <string>
#include <memory>

SAIGON_NAMESPACE_BEGIN
class bstar
{
    inline static constexpr int STAR_HEARTBEAT = 1000;
    //  States we can be in at any point in time
	enum class state_t : int
	{
        STATE_PRIMARY   = 1, //  Primary, waiting for peer to connect
        STATE_BACKUP    = 2, //  Backup, waiting for peer to connect
        STATE_ACTIVE    = 3, //  Active - accepting connections
        STATE_PASSIVE   = 4//  Passive - not accepting connections
	};

    //  Events, which start with the states our peer can be in
    enum class event_t : int
    {
        PEER_PRIMARY    = 1, //  HA peer is pending primary
        PEER_BACKUP     = 2, //  HA peer is pending backup
        PEER_ACTIVE     = 3, //  HA peer is active
        PEER_PASSIVE    = 4, //  HA peer is passive
        CLIENT_REQUEST  =5 //  Client makes request
    };

    zmqpp::context_t& ctx_;
    std::unique_ptr<zmqpp::loop> loop_;
    std::unique_ptr<zmqpp::socket_t> statepub_;
    std::unique_ptr<zmqpp::socket_t> statesub_;
    std::unique_ptr<zmqpp::socket_t> server_;

    state_t state_;
    event_t event_;
    std::chrono::steady_clock::time_point peer_expiry_;
    std::string frontend_host_;

public:
    // callback
    using Callable = std::function<bool(bstar*)>;
    Callable active_fn_{ nullptr };
    Callable passive_fn_{ nullptr };

public:
    bstar(zmqpp::context_t& ctx, std::string_view local, std::string_view remote, bool primary);
    void start();
    void voter(std::string_view endpoint, zmqpp::socket_type type, Callable callback);

    void new_active(Callable fn);
    void new_passive(Callable fn);
    void update_peer_expiry();
    zmqpp::socket_t* get_frontend() const;
    std::string get_frontend_host() const;

private:
    void init(std::string_view local, std::string_view remote);
    bool send_state(zmqpp::socket_t* sock);
    bool recv_state(zmqpp::socket_t* sock);
    bool execute();
};
SAIGON_NAMESPACE_END