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

// MMI - Majordomo Management Interface
const std::string MMI_SERVICE			= "mmi.service";
const std::string MMI_FOUND				= "200";
const std::string MMI_NOT_FOUND			= "404";
const std::string MMI_NOT_IMPLEMENTED	= "501";
const std::string MMI_KEY				= "mmi.";

// TSP - Titanic Service Protocol
const std::string TITANIC_REQUEST	= "titanic.request";
const std::string TITANIC_REPLY		= "titanic.reply";
const std::string TITANIC_CLOSE		= "titanic.close";
const std::string TITANIC_DIR		= ".titanic";
const std::string QUEUE_FILE		= "queue";
const std::string QUEUE_LINEFORMAT	= "-{0}\n";