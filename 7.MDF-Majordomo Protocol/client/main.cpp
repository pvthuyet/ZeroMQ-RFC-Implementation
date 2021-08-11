#include <vld.h>
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
		if (response.parts() == 0) {
			SPDLOG_ERROR("Request {} doesn't work", MMI_SERVICE);
			return EXIT_FAILURE;
		}
		else {
			auto resmsg = response.get<std::string>(0);
			if (resmsg != MMI_FOUND) {
				SPDLOG_INFO("Service '{}' is not available {}", service_name, resmsg);
				return EXIT_FAILURE;
			}
		}

		for (int i = 1; i < 10; ++i) {
			auto sendmsg = std::format("hello {}", i);
			client.send(service_name, sendmsg);
			auto reply = client.recv();
			auto repmsg = reply.get<std::string>(0);
			std::cout << sendmsg << " => " << repmsg << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}

	return EXIT_SUCCESS;
}