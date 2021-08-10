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
		if (argc < 3) {
			SPDLOG_INFO("Usage: mdworker [service_name] [config.json]");
			return EXIT_FAILURE;
		}

		// Load config file
		std::string service_name = argv[1];
		sg::config_parser conf;
		conf.load(argv[2]);

		zmqpp::context_t ctx;
		mdcliapi client(ctx, conf.get_value(BACKEND_ENDPOINT));

		// Discovery service
		client.send(MMI_SERVICE, service_name);
		auto response = client.recv();
		if (response != MMI_FOUND) {
			SPDLOG_INFO("Service '{}' is not available {}", service_name, response);
			return EXIT_FAILURE;
		}

		while (true) {
			std::cout << "Enter message: ";
			std::string msg;
			std::cin >> msg;
			if (boost::iequals(msg, "q"s)) break;
			client.send(service_name, msg);
			auto reply = client.recv();
			std::cout << reply << std::endl;
		}
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}

	return EXIT_SUCCESS;
}