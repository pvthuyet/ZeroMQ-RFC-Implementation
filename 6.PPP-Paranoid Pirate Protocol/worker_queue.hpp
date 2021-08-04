#pragma once

#include <string>
#include <chrono>
#include <vector>
#include "define.hpp"

#define HEARTBEAT_LIVENESS  3       //  3-5 is reasonable
#define HEARTBEAT_INTERVAL  1000    //  msecs

SAIGON_NAMESPACE_BEGIN
class worker_queue
{
private:
	struct worker_t
	{
		std::string identity_;
		std::chrono::steady_clock::time_point when_;
	};
	std::vector<worker_t> queue_;

public:
	size_t size() const;
	const std::vector<worker_queue::worker_t>& data() const;
	void push(std::string_view identity);
	void erase(std::string_view identity);
	void refresh(std::string_view identity);
	std::string pop();
	void purge();
};
SAIGON_NAMESPACE_END