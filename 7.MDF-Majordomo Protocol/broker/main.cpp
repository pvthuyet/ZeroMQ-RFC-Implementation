#include <vld.h>
#include "mdbroker.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"
#include "utils/config_parser.hpp"

int main(int argc, char* argv[])
{
	using namespace sg;
	logger::get_instance();
	try {
		if (argc < 2) {
			SPDLOG_INFO("Usage: mdworker config.json");
			return EXIT_FAILURE;
		}

		// Load config file
		sg::config_parser conf;
		conf.load(argv[1]);

		zmqpp::context_t ctx;
		mdbroker broker(ctx, 
			conf.get_value(BACKEND_HOST),
			conf.get_value(ADMIN_ENDPOINT)
		);

		broker.start();
		broker.wait();
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}


	return EXIT_SUCCESS;
}