#include "echoworker.hpp"
#include "mdwrkapi.hpp"
#include "logger/logger_define.hpp"

SAIGON_NAMESPACE_BEGIN

echoworker::echoworker(zmqpp::context_t& ctx,
	std::string_view brokerep,
	std::string_view adminep,
	std::string_view identity
) :
	mdworker(ctx, brokerep, adminep, "echo", identity)
{}

void echoworker::run(std::stop_token tok)
{
	LOGENTER;
	try {
		session_.connect_to_broker();
		while (true) {
			auto req = session_.recv(tok);
			if (req) {
				session_.send(std::move(req));
			}
		}
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
	}
	LOGEXIT;
}

SAIGON_NAMESPACE_END