#include <vld.h>
#include "echoworker.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"
#include "utils/config_parser.hpp"

int main(int argc, char* argv[])
{
	using namespace sg;
	logger::get_instance();
	try {
		if (argc < 2) {
			SPDLOG_INFO("Usage: ppworker config.json");
			return EXIT_FAILURE;
		}

		// Load config file
		sg::config_parser conf;
		conf.load(argv[1]);

		zmqpp::context_t ctx;
		sg::echoworker echowrk(ctx,
			conf.get_value("backend_endpoint"),
			conf.get_value("admin_endpoint")
		);
		echowrk.start();
		echowrk.wait();
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}


	return EXIT_SUCCESS;
}