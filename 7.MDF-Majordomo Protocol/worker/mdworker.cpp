#include "mdworker.hpp"
#include "logger/logger_define.hpp"
#include "admin/sub_admin.hpp"

SAIGON_NAMESPACE_BEGIN
mdworker::mdworker(zmqpp::context_t& ctx,
	std::string_view brokerep,
	std::string_view adminep,
	std::string_view service_name,
	std::string_view identity
	):
	iworker(ctx, adminep),
	session_(ctx, brokerep, service_name, identity)
{}

mdworker::~mdworker() noexcept = default;

SAIGON_NAMESPACE_END