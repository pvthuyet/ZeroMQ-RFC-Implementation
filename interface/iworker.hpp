#pragma once

#include "define.hpp"
#include "admin/sub_admin.hpp"

SAIGON_NAMESPACE_BEGIN
class iworker
{
private:
	std::unique_ptr<std::jthread> thread_;
	sub_admin admin_;

public:
	iworker(zmqpp::context_t& ctx,
		std::string_view adminep
	);
	virtual ~iworker() noexcept;

	void start();
	void wait();

private:
	virtual void run(std::stop_token tok) = 0;
};
SAIGON_NAMESPACE_END