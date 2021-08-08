#pragma once

#include <string>

//  Reliability parameters
constexpr int HEARTBEAT_LIVENESS	= 3;       //  3-5 is reasonable
constexpr int HEARTBEAT_INTERVAL	= 2500;
constexpr int HEARTBEAT_EXPIRY		= HEARTBEAT_INTERVAL * HEARTBEAT_LIVENESS;

// For config file
const std::string ADMIN_HOST		= "admin_host";
const std::string ADMIN_ENDPOINT	= "admin_endpoint";
const std::string FRONTEND_HOST		= "frontend_host";
const std::string BACKEND_HOST		= "backend_host";
const std::string FRONTEND_ENDPOINT = "frontend_endpoint";
const std::string BACKEND_ENDPOINT	= "backend_endpoint";
const std::string SERVICES			= "services";