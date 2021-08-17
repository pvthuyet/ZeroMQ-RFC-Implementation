#include <vld.h>
#include "bstar.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"
#include "utils/config_parser.hpp"
#include "bsp_constant.hpp"
#include <boost/algorithm/string.hpp>

class echo_server
{
public:
	bool operator()(sg::bstar* api)
	{
		auto socket = api->get_frontend();
		zmqpp::message_t msg;
		socket->receive(msg);
		socket->send(msg);
		return true;
	}
};

int main(int argc, char* argv[])
{
	using namespace std::string_literals;
	using namespace sg;
	logger::get_instance();
	try {
		if (argc < 3) {
			SPDLOG_INFO("Usage: bstarsrv1 {-p | -b} [config.json]");
			return EXIT_FAILURE;
		}

		//
		bool isprimary = boost::iequals(argv[1], "-p");
		// Load config file
		std::string service_name = argv[1];
		sg::config_parser conf;
		conf.load(argv[2]);

		zmqpp::context_t ctx;

		std::string fehost;
		std::string statepub_host;
		std::string statesub_ep;
		if (isprimary) {
			fehost = conf.get_value(frontend_host);
			statepub_host = conf.get_value(state_publish_host);
			statesub_ep = conf.get_value(backup_state_publish_host_endpoint);
		}
		else {
			fehost = conf.get_value(backup_frontend_host);
			statepub_host = conf.get_value(backup_state_publish_host);
			statesub_ep = conf.get_value(state_publish_host_endpoint);
		}

		sg::bstar server(
			ctx,
			statepub_host,
			statesub_ep,
			isprimary
		);

		server.new_active([](bstar* api) {
			SPDLOG_INFO("Active: {}", api->get_frontend_host());
			return true;
			});

		server.new_passive([](bstar* api) {
			SPDLOG_INFO("Passive: {}", api->get_frontend_host());
			return true;
			});

		server.voter(fehost, zmqpp::socket_type::router, echo_server{});
		server.start();
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}
	return EXIT_SUCCESS;
}