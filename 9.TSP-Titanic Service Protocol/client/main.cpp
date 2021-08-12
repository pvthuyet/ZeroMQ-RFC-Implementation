#include <vld.h>
#include "ticlient.hpp"
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
		std::string srvname = argv[1];
		sg::config_parser conf;
		conf.load(argv[2]);

		zmqpp::context_t ctx;
		ticlient client(ctx, conf.get_value(BACKEND_ENDPOINT));
		zmqpp::message_t msg("hello world");
		auto uuid = client.send(srvname, msg);
		auto reply = client.recv(uuid);
		SPDLOG_INFO("Reply: {}", reply.get<std::string>(reply.parts() - 1));
	}
	catch (const std::exception& ex) {
		SPDLOG_ERROR(ex.what());
	}
	return EXIT_SUCCESS;
}