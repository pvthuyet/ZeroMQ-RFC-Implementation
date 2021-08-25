#pragma once

#include "define.hpp"

SAIGON_NAMESPACE_BEGIN

constexpr int PING_INTERVAL		= 2000; // in msecs
constexpr int SERVER_TTL		= 6000; // in msecs
constexpr int GLOBAL_TIMEOUT	= 3000; // in msecs
constexpr int INTERVAL			= 1000; // in msecs

const std::string admin_host		= "admin_host";
const std::string admin_endpoint	= "admin_endpoint";
const std::string server_host		= "server_host";
const std::string server_endpoint	= "server_endpoint";

const std::string server_host2		= "server_host2";
const std::string server_endpoint2  = "server_endpoint2";

const std::string server_host3		= "server_host3";
const std::string server_endpoint3  = "server_endpoint3";

const std::string COMMAND_CONNECT	= "CONNECT";
const std::string COMMAND_REQUEST	= "REQUEST";
const std::string COMMAND_PING		= "PING";
const std::string COMMAND_PONG		= "PONG";

SAIGON_NAMESPACE_END