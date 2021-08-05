#include "logger/logger.hpp"
#include "logger/logger_define.hpp"
#include <vld.h>
#include "pub_admin.hpp"
#include <sgutils/json_reader.hpp>

int main(int argc, char* argv[])
{
	using namespace sg;
	logger::get_instance();
	try {
		if (argc < 2) {
			SPDLOG_INFO("Usage: admin config.json");
			return EXIT_FAILURE;
		}

		// Load config file
		sg::json_reader reader{};
		reader.read(argv[1]);

		zmqpp::context_t ctx;
		sg::pub_admin admin(ctx, reader.get<std::string>("admin_host"));
		admin.start();
	}
	catch (std::exception const& ex) {
		SPDLOG_ERROR(ex.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

}