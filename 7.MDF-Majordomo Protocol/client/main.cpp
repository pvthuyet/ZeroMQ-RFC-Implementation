#include "mdcliapi.hpp"
#include "utils/config_parser.hpp"
#include "constant.hpp"
#include "logger/logger.hpp"
#include "logger/logger_define.hpp"
#include <boost/algorithm/string.hpp>

int main(int argc, char* argv[])
{
	using namespace std::string_literals;
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
		mdcliapi client(ctx, conf.get_value(BACKEND_ENDPOINT));

		while (true) {
			std::cout << "Enter message: ";
			std::string msg;
			std::cin >> msg;
			if (boost::iequals(msg, "q"s)) break;
			client.send("echo", msg);
			auto reply = client.recv();
			std::cout << reply << std::endl;
		}
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}

	return EXIT_SUCCESS;
}