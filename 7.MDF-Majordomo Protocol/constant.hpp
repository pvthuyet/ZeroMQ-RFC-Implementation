#pragma once

//  Reliability parameters
constexpr int HEARTBEAT_LIVENESS = 5;       //  3-5 is reasonable
constexpr int HEARTBEAT_DELAY = 2500;

// For config file
const std::string ADMIN_HOST		= "tcp://*:5557";
const std::string ADMIN_ENDPOINT	= "tcp://localhost:5557";
const std::string FRONTEND_HOST		= "tcp://*:5555";
const std::string BACKEND_HOST		= "tcp://*:5556";
const std::string FRONTEND_ENDPOINT = "tcp://localhost:5555";
const std::string BACKEND_ENDPOINT	= "tcp://localhost:5556";
const std::string SERVICES			= "services";