#include <vld.h>
#include "ppworker.hpp"
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
		ppworker worker(ctx, 
			reader.get<std::string>("backend_endpoint"), 
			reader.get<std::string>("admin_endpoint"),
			argv[1]);
		worker.start();
		worker.wait();
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}