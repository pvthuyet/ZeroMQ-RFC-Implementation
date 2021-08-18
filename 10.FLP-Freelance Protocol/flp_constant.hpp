#pragma once

#include "define.hpp"

SAIGON_NAMESPACE_BEGIN

constexpr int PING_INTERVAL		= 2000; // in msecs
constexpr int SERVER_TTL		= 6000; // in msecs
constexpr int GLOBAL_TIMEOUT	= 3000; // in msecs
constexpr int HEARTBEAT			= 1000; // in msecs

const std::string admin_host		= "admin_host";
const std::string admin_endpoint	= "admin_endpoint";
const std::string server_host		= "server_host";
const std::string server_endpoint	= "server_endpoint";

const std::string COMMAND_CONNECT	= "CONNECT";
const std::string COMMAND_REQUEST	= "REQUEST";

SAIGON_NAMESPACE_END