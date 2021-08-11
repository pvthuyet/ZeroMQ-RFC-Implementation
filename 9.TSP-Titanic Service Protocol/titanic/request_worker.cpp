#include "request_worker.hpp"
#include "logger/logger_define.hpp"
#include "sgutils/random_factor.hpp"
#include "utils/zmqutil.hpp"
#include <filesystem>

SAIGON_NAMESPACE_BEGIN

request_worker::request_worker(
	zmqpp::socket_t* pipe,
	zmqpp::context_t& ctx,
	std::string_view brokerep,
	std::string_view adminep,
	std::string_view identity
) :
	mdworker(ctx, brokerep, adminep, request_worker::NAME, identity),
	pipe_{pipe}
{}

std::string request_filename(std::string_view uuid)
{
	return std::format("{}/{}.req", TITANIC_DIR, uuid);
}

void request_worker::run(std::stop_token tok)
{
	//Frame 0: Service name(printable string)
	//Frames 1 + : Request body(opaque binary)
	LOGENTER;
	try {
		session_.connect_to_broker();
		while (true) {
			auto req = session_.recv(tok);
			if (req) {
				// Ensure message directory exists
				std::filesystem::create_directories(TITANIC_DIR);

				// Generate UUID and save message to disk
				auto uuid = sg::random_factor{}.ramdom_uuid<std::string>();

				// save to disk
				zmqutil::save(request_filename(uuid), *req);

				// Send UUID through to message queue
				pipe_->send(uuid);

				// Now send UUID back to client
				auto reply = std::make_optional<zmqpp::message_t>();
				reply->push_back(MMI_FOUND);
				reply->push_back(uuid);
				session_.send(std::move(reply));
			}
		}
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
	}
	LOGEXIT;
}
SAIGON_NAMESPACE_END