#include "worker_queue.hpp"
#include <gsl/gsl_assert>
#include <algorithm>

namespace saigon {

void worker_queue::push(std::string_view identity)
{
	auto it = std::ranges::find_if(queue_, [identity](auto const& item) {
		return item.identity_ == identity;
		});
	if (it == std::cend(queue_)) {
		 queue_.push_back(worker_t { 
			 .identity_ = std::string{identity}, 
			 .expiry_ = std::chrono::steady_clock::now()
			 });
	}
}

void worker_queue::erase(std::string_view identity)
{
	std::erase_if(queue_, [identity](auto const& item) {
		return item.identity_ == identity;
		});
}

void worker_queue::refresh(std::string_view identity)
{
	auto it = std::ranges::find_if(queue_, [identity](auto const& item) {
		return item.identity_ == identity;
		});
	if (it != std::cend(queue_)) {
		it->expiry_ = std::chrono::steady_clock::now();
	}
}

std::string worker_queue::pop()
{
	Ensures(queue_.size());
	std::string identity = queue_.front().identity_;
	queue_.erase(queue_.begin());
	return identity;
}

// look for & kill expired workers
void worker_queue::purge()
{
	constexpr long long exp = HEARTBEAT_LIVENESS * HEARTBEAT_INTERVAL;
	auto now = std::chrono::steady_clock::now();
	std::erase_if(queue_, [&now, exp](auto const& item) {
		auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - item.expiry_);
		return diff.count() > exp;
		});
}

}