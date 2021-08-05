//#include <vld.h>
#include "lpclient.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"
#include <sgutils/json_reader.hpp>

int main(int argc, char* argv[])
{
	using namespace sg;
	logger::get_instance();
	try {
		if (argc < 2) {
			SPDLOG_INFO("Usage: lpclient config.json");
			return EXIT_FAILURE;
		}

		// Load config file
		sg::json_reader reader;
		reader.read(argv[1]);

		zmqpp::context_t ctx;
		lpclient client(ctx, reader.get<std::string>("frontend_endpoint"));
		client.start();
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}