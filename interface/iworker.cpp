#include "iworker.hpp"
#include "logger/logger_define.hpp"

SAIGON_NAMESPACE_BEGIN
iworker::iworker(zmqpp::context_t& ctx,
	std::string_view adminep
) :
	thread_{},
	admin_(ctx, adminep)
{}

iworker::~iworker() noexcept = default;

void iworker::start()
{
	LOGENTER;
	if (thread_) {
		SPDLOG_ERROR("Worker is already started");
		return;
	}

	SPDLOG_INFO("Starting worker...");
	thread_ = std::make_unique<std::jthread>([this](std::stop_token tok) {
		this->run(tok);
		});
	admin_.start();
	LOGEXIT;
}

void iworker::wait()
{
	admin_.wait();
}

SAIGON_NAMESPACE_END