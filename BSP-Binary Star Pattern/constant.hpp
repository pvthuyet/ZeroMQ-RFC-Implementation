#pragma once

#include "define.hpp"
#include <chrono>

SAIGON_NAMESPACE_BEGIN

//  States we can be in at any point in time
enum class state_t : int
{
	none,
	primary,	//  Primary, waiting for peer to connect
	backup,		//  Backup, waiting for peer to connect
	active,		//  Active - accepting connections
	passive		//  Passive - not accepting connections
};

enum class event_t : int
{
	none,
	peer_primary,		//  HA peer is pending primary
	peer_backup,		//  HA peer is pending backup
	peer_active,		//  HA peer is active
	peer_passive,		//  HA peer is passive
	client_request		//  Client makes request
};

// Our finite state machine
struct bstar_t {
	state_t state_{ state_t::none };		// Current state
	event_t event_{ event_t::none };		// Current event
	std::chrono::steady_clock::time_point peer_exipry_;	// When peer is considered 'dead'
};

constexpr int HEARTBEAT = 1000; // in msecs
const std::string admin_host							= "admin_host";
const std::string admin_endpoint						= "admin_endpoint";
const std::string frontend_host							= "frontend_host";
const std::string frontend_endpoint						= "frontend_endpoint";
const std::string state_publish_host					= "state_publish_host";
const std::string state_publish_host_endpoint			= "state_publish_host_endpoint";
const std::string state_subscribe_host					= "state_subscribe_host";
const std::string state_subscribe_host_endpoint			= "state_subscribe_host_endpoint";
const std::string backup_frontend_host					= "backup_frontend_host";
const std::string backup_frontend_endpoint				= "backup_frontend_endpoint";
const std::string backup_state_publish_host				= "backup_state_publish_host";
const std::string backup_state_publish_host_endpoint	= "backup_state_publish_host_endpoint";
const std::string backup_state_subscribe_host			= "backup_state_subscribe_host";
const std::string backup_state_subscribe_host_endpoint	= "backup_state_subscribe_host_endpoint";

SAIGON_NAMESPACE_END