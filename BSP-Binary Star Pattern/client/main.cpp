#include <vld.h>
#include "bstarcli.hpp"
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
		if (argc < 2) {
			SPDLOG_INFO("Usage: bstarcli [config.json]");
			return EXIT_FAILURE;
		}

		// Load config file
		sg::config_parser conf;
		conf.load(argv[1]);

		zmqpp::context_t ctx;
		bstarcli client(ctx);
		client.add_server(conf.get_value(frontend_endpoint));
		client.add_server(conf.get_value(backup_frontend_endpoint));
		client.start();
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}
	return EXIT_SUCCESS;
}