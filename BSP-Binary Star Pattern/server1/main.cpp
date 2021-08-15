#include <vld.h>
#include "binary_star_server1.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"
#include "utils/config_parser.hpp"
#include "bsp_constant.hpp"
#include <boost/algorithm/string.hpp>

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

		binary_star_server1 server(ctx,
			fehost,
			statepub_host,
			statesub_ep
		);
		server.start(isprimary);
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}
	return EXIT_SUCCESS;
}