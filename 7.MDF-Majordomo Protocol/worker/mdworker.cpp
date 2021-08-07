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
	adminep_{adminep},
	session_(ctx, brokerep, service_name, identity)
{}

mdworker::~mdworker() noexcept = default;

void mdworker::start()
{
	LOGENTER;
	if (thread_) {
		SPDLOG_ERROR("Worker {} {} is already started", session_.get_service_name(), session_.get_id());
		return;
	}

	SPDLOG_INFO("Starting worker {} {}", session_.get_service_name(), session_.get_id());
	thread_ = std::make_unique<std::jthread>([this](std::stop_token tok) {
		this->session_.connect_to_broker();
		this->run(tok);
		});
	LOGEXIT;
}

void mdworker::wait()
{
	// Subscribe the admin
	sg::sub_admin admin(session_.get_context(), adminep_);
	admin.wait();
}

SAIGON_NAMESPACE_END