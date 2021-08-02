#pragma once

#include <string>
#include <chrono>
#include <vector>

#define HEARTBEAT_LIVENESS  3       //  3-5 is reasonable
#define HEARTBEAT_INTERVAL  1000    //  msecs

namespace saigon {
class worker_queue
{
private:
	struct worker_t
	{
		std::string identity_;
		std::chrono::steady_clock::time_point expiry_;
	};
	std::vector<worker_t> queue_;

public:
	void push(std::string_view identity);
	void erase(std::string_view identity);
	void refresh(std::string_view identity);
	std::string pop();
	void purge();
};
}