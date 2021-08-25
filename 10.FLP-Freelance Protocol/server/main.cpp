#include <vld.h>
#include "flserver.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"
#include "utils/config_parser.hpp"
#include "flp_constant.hpp"
#include <boost/algorithm/string.hpp>

int main(int argc, char* argv[])
{
	using namespace std::string_literals;
	using namespace sg;
	logger::get_instance();
	try {
		if (argc < 3) {
			SPDLOG_INFO("Usage: flserver [config.json] [index]");
			return EXIT_FAILURE;
		}

		// Load config file
		sg::config_parser conf;
		conf.load(argv[1]);
		int idx = atoi(argv[2]);
		zmqpp::context_t ctx;

		std::string host;
		std::string endpoint;
		host = conf.get_value(server_host);
		endpoint = conf.get_value(server_endpoint);
		if (idx == 2) {
			host = conf.get_value(server_host2);
			endpoint = conf.get_value(server_endpoint2);
		}
		else if (idx == 3){
			host = conf.get_value(server_host3);
			endpoint = conf.get_value(server_endpoint3);
		}
		sg::flserver server(ctx, host, endpoint);
		server.start();
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}
	return EXIT_SUCCESS;
}