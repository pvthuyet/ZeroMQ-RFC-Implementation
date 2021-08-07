#include <vld.h>
#include "echoworker.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"
#include "utils/config_parser.hpp"
#include "mdworker_factory.hpp"

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
		auto wrks = mdworker_factory{}.create(ctx, conf);

		// start all workers
		for (auto& w : wrks) w->start();

		// Wait to exit
		for (auto& w : wrks) w->wait();
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}


	return EXIT_SUCCESS;
}