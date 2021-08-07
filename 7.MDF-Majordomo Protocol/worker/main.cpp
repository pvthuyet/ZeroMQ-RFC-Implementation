#include <vld.h>
#include "echoworker.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"
#include <sgutils/json_reader.hpp>

int main(int argc, char* argv[])
{
	using namespace sg;
	logger::get_instance();
	try {
		if (argc < 3) {
			SPDLOG_INFO("Usage: ppworker 12345 config.json");
			return EXIT_FAILURE;
		}

		// Load config file
		sg::json_reader reader;
		reader.read(argv[2]);

		zmqpp::context_t ctx;
		sg::echoworker echowrk(ctx,
			reader.get<std::string>("backend_endpoint"),
			reader.get<std::string>("admin_subscriber"),
			argv[1]
		);
		echowrk.start();
		echowrk.wait();
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}


	return EXIT_SUCCESS;
}