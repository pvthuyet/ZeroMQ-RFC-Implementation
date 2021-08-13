#include "reply_worker.hpp"
#include "logger/logger_define.hpp"
#include "sgutils/random_factor.hpp"
#include "utils/zmqutil.hpp"
#include <filesystem>

SAIGON_NAMESPACE_BEGIN

reply_worker::reply_worker(
	zmqpp::context_t& ctx,
	std::string_view brokerep,
	std::string_view adminep,
	std::string_view identity
) :
	mdworker(ctx, brokerep, adminep, reply_worker::NAME, identity)
{}

void reply_worker::run(std::stop_token tok)
{
	namespace fs = std::filesystem;
	//Frame 0: Service name(printable string)
	//Frames 1 + : Request body(opaque binary)
	LOGENTER;
	try {
		session_.connect_to_broker();
		while (true) {
			auto req = session_.recv(tok);
			if (req) {
				zmqpp::message_t reply;
				auto uuid = req->get<std::string>(0);
				auto repfn = std::format("{}/{}.rep", TITANIC_DIR, uuid);
				auto ab = fs::absolute(repfn);
				if (fs::exists(repfn)) {
					reply = zmqutil::load(repfn);
					reply.push_front("200");
				}
				else {
					auto reqfn = std::format("{}/{}.req", TITANIC_DIR, uuid);
					if (fs::exists(reqfn)) {
						reply.push_back("300");
					}
					else {
						reply.push_back("400");
					}
				}
				session_.send(std::make_optional<zmqpp::message_t>(std::move(reply)));
			}
		}
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
	}
	LOGEXIT;
}
SAIGON_NAMESPACE_END