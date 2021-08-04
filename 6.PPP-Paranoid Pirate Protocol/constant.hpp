#pragma once

constexpr int HEARTBEAT_LIVENESS	= 3;		//  3-5 is reasonable
constexpr int HEARTBEAT_INTERVAL	= 1000;		//  msecs
constexpr int INTERVAL_INIT			= 1000;		//  Initial reconnect
constexpr int INTERVAL_MAX			= 32000;	//  After exponential backoff
