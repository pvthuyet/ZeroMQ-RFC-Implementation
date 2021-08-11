#include "titanic.hpp"
#include "utils/config_parser.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"

int main(int argc, char* argv[])
{
	using namespace std::string_literals;
	using namespace sg;
	logger::get_instance();
	try {
		if (argc < 2) {
			SPDLOG_INFO("Usage: titanic [config.json]");
			return EXIT_FAILURE;
		}

		// Load config file
		std::string service_name = argv[1];
		sg::config_parser conf;
		conf.load(argv[1]);

		zmqpp::context_t ctx;
		titanic tita(ctx, 
			conf.get_value(BACKEND_ENDPOINT),
			conf.get_value(ADMIN_ENDPOINT));

		tita.start();
		tita.wait();
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}
	return EXIT_SUCCESS;
}